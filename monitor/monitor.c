#include "../misc/config.h"
#include "altio.h"
#include "base.h"
#include "capman.h"
#include "payload.h"
#include "s3k.h"
#include "aes128.h"
#include "code-auth.h"

#include <stdbool.h>
#include <string.h>

#define SECURITY

#define MONITOR_PID 0
#define APP_PID 1

#define SIGNATURE_SIZE 16 // bytes
#define HEADER_SIZE 512 // bytes

#define SOCKET_PORT1 0xa
#define SOCKET_PORT2 0xb

struct memory_section {
	char section[8];
	uint64_t offset;
	uint64_t size;
	uint8_t rwx;
};

struct process {
	uint8_t pid;
	uint64_t header_len;
	struct memory_section header[8];
};

bool app_running = false;

uint8_t pmpcaps[8] = { 0 };
uint8_t soccaps[8] = { 0 };

uint64_t recv_buf[4] = { 0x0, 0x0, 0x0, 0x0 };
uint64_t tag;

/* this shit is so janky */
struct process parse_executable(uint8_t pid, void * base_addr)
{
	struct process proc;

	/* copy over pid */
	proc.pid = pid;

	/* initialize header length to 0 */
	proc.header_len = 0;

	uint64_t * p = (uint64_t*)base_addr;
	p += SIGNATURE_SIZE / 8; // skip signature

	/* parse header */
	for (int i = 0; i < 8; i++) {
		if (*p == 0) { // Check if end of header reached
			break;
		}

		/* parse section name */
		uint8_t * pn = (uint8_t*)p++;
		for (int j = 0; j < 8; j++) {
			if (i == 7) { // set last byte in buffer to NULL byte to terminate string.
				proc.header[i].section[j] =  0;
				break;
			}

			proc.header[i].section[j] =  *pn++;

			if (proc.header[i].section[j] == 0) { // if NULL byte is encountered -> end of string.
				break;
			}
		}

		/* copy over setion offset, size and permissions */
		proc.header[i].offset = *p++;
		proc.header[i].size = *p++;
		proc.header[i].rwx = *p++; // change later to *p++
		proc.header_len++;
	}

	return proc;
}

void setup_time_slices(void) 
{
	uint64_t hartid = 0;
	/* App time */
	capman_derive_time(0x18, hartid, 0, 32);
}

void setup_ipc(void)
{	
	capman_derive_socket(SOCKET_PORT1, 0, 0);
	capman_derive_socket(0x20, 0, 1);
	capman_mgivecap(APP_PID, 0x20, SOCKET_PORT2);
	capman_derive_socket(0x20, 1, 0);
	capman_derive_socket(SOCKET_PORT2, 1, 1);
	capman_mgivecap(APP_PID, 0x20, SOCKET_PORT1);
}

void setup_monitor(void)
{
	/* Give monitor access to UART memory */
	capman_derive_pmp(0x09, (uint64_t)UART_BASE, (uint64_t)UART_BASE + 0x100, S3K_RW);
	pmpcaps[2] = 0x09;
	
	/* Give monitor access to APP memory */
	capman_derive_pmp(0xc, APP_BASE, APP_BASE + APP_SIZE, S3K_RW);
	pmpcaps[1] = 0xc;
	capman_setpmp(pmpcaps);


	capman_setpmp(pmpcaps);
}

void setup_app(struct process app) 
{
	/* Keep track of PMP capabilities created */
	uint8_t npmp = 0; // number of PMP capabilities created
	uint64_t ipmp = 0x0000000000000000; // byte mask of indices to PMP capabilities

#ifdef SECURITY
	/* Derive PMP for each section and give to APP */
	for (uint8_t j = 0; j < app.header_len; j++) {
		if (strcmp(app.header[j].section, ".heap") == 0) {
			uint64_t nchunks = app.header[j].size / CHUNK_SIZE;
			for (uint64_t k = 0; k < nchunks; k += nchunks-1) { // only create pmp for first and last heap chunk only to be a proof of concept
				capman_derive_pmp(0x20, APP_BASE + app.header[j].offset + (k*CHUNK_SIZE), APP_BASE + app.header[j].offset + (k*CHUNK_SIZE) + CHUNK_SIZE, app.header[j].rwx);
				capman_mgivecap(APP_PID, 0x20, npmp++);
			}
			continue;
		} 
		capman_derive_pmp(0x20, APP_BASE + app.header[j].offset, APP_BASE + app.header[j].offset + app.header[j].size, app.header[j].rwx);
		capman_mgivecap(APP_PID, 0x20, npmp++);
	} 
#else
	capman_derive_pmp(0x20, APP_BASE, APP_BASE + APP_SIZE, S3K_RWX);
	capman_mgivecap(APP_PID, 0x20, npmp++);
#endif

	/* Create capability to access UART memory, then give it to APP */
	capman_derive_pmp(0x20, (uint64_t)UART_BASE, (uint64_t)UART_BASE + 0x100, S3K_RW);
	capman_mgivecap(APP_PID, 0x20, npmp++);
	
	/* Give APP time slice */
	capman_mgivecap(APP_PID, 0x18, 0xF);

	/* Set APP PC */
	capman_msetreg(APP_PID, S3K_REG_PC, APP_BASE);

	/* Set APP PMP */
	for (uint8_t j = (npmp - 1); j != 0xFF; j--) {
		ipmp += j;
		ipmp <<= (j != 0x00) ? 0x08 : 0x00; /* shift one byte position to the left when j is not 0x00  */
	}
	capman_msetreg(APP_PID, S3K_REG_PMP, ipmp);
}

void load_app()
{	
	/* Copy app-binary to allocated memory: */
	memcpy((void *)APP_BASE, (void*)(app_bin + HEADER_SIZE), app_bin_len - HEADER_SIZE);
}

void setup(void)
{
	struct process app; //struct process app;

	/* Delete time on core 2 3 4. */
	s3k_delcap(4);
	s3k_delcap(5);
	s3k_delcap(6);

	/* Initialize capman */
	capman_init();

	/* Setup monitor */
	setup_monitor();
	alt_printf("\n[MON]\tSetup\n");

	/* Setup memory and time */
	alt_printf("\t- setup time\n");
	setup_time_slices();

	/* Parse app */
	alt_printf("\t- parsing app\n");
	app = parse_executable(APP_PID, (void*)app_bin);

#ifdef SECURITY
	uint8_t signature[16]; // Holder for calculating MAC signature
	uint8_t * ptr_app;

	/* Calculate mac of app and store in signature */
	ptr_app = (void *)(app_bin + HEADER_SIZE);
	calc_sig(ptr_app, app_bin_len - HEADER_SIZE, signature);
  
	alt_printf("\t- verifying app... ");
  	/* Authentication, compares provided signature with calculated signature and setup app if successfull */
	if (comp_sig((void *)app_bin, signature) != 1){
    	alt_printf("FAIL!\n\t- Log: unmatching signature\n\t- Action: app will not be started\n");
		return;
  	}
    	
	alt_printf("SUCCESS!\n");
#endif /* SECURITY */

  	/* Load app to allocated memory */
	alt_printf("\t- load app\n");
	load_app();
  
	/* Setup app */
	alt_printf("\t- setup app\n");
	setup_app(app);

	/* Setup IPC */
	alt_printf("\t- setup ipc\n");
	setup_ipc();
	capman_getsoc(soccaps);

	alt_printf("\t- caps{\n");
	capman_dump_all();
	alt_printf("}\n");

	s3k_yield();

	/* Start app */
	alt_printf("\n[MON]\tResuming app...\n");
	capman_mresume(APP_PID);

	/* Wait for app to respond */
	s3k_recv(soccaps[0], recv_buf, -1ull, &tag);
	if (recv_buf[0] != -1ull) {
		alt_printf("\n[MON]\tApp setup failed! Received: 0x%X\n", recv_buf[0]);
		return;
	}
	alt_printf("\n[MON]\tApp setup success! Received: 0x%X\n", recv_buf[0]);
	app_running = true;
}

void loop(void)
{
	static uint8_t iloop = 0;
	union s3k_cap pmp;
	uint64_t err;

	alt_printf("\n[MON]\tLoop #0x%X\n", iloop++);

	while (!(app_running)) {
		s3k_yield();
	}

	/* Yields until msg read from buffer */	
	s3k_recv(soccaps[0], recv_buf, 0x30, &tag);

	switch (recv_buf[0]) {
		case -1ull:
			alt_printf("\n[MON]\tApp finished execution, nothing more to do...\n");
			app_running = false;
			break;
		case 0x00:
			alt_printf("\n[MON]\tApp successfully handled an exception\n");
			break;
		case 0x01:
			alt_printf("\n[MON]\tApp failed to handle an exception\n");
			break;
		case 0x02:
			alt_printf("\n[MON]\tApp requests to update PMP permissions\n");
			capman_update();

			pmp = capman_get(0x30);

			/* Check if capability sent is a pmp */
			if (pmp.type != S3K_CAPTY_PMP) {
				alt_printf("\t- Log: capability received is of type (0x%X), not pmp\n", pmp.type);
				recv_buf[0] = 0x02; // unexpected capability type
				s3k_send(soccaps[1], recv_buf, 0x30, false);
				capman_update();
				break;
			}

			uint64_t begin = s3k_pmp_napot_begin(pmp.pmp.addr);
			uint64_t end = s3k_pmp_napot_end(pmp.pmp.addr);
			uint64_t rwx = pmp.pmp.cfg & 0x7;
			alt_printf("\t- Log: old pmp{begin=0x%X,end=0x%X,rwx=0x%X}\n", begin, end, rwx);
			alt_printf("\t- Log: new pmp{begin=0x%X,end=0x%X,rwx=0x%X}\n", begin, end, recv_buf[3]);

#ifdef SECURITY
			/* Check if both W and X permissions are requested */	
			if (recv_buf[3] & 0x02 && recv_buf[3] & 0x04) { 
				alt_printf("\t- Log: W and X permissions forbidden\n", recv_buf[3]);
				recv_buf[0] = 0x03; // forbidden capability
				s3k_send(soccaps[1], recv_buf, 0x30, false);
				capman_update();
				break;
			}

			/* Check if X permission is requested */
			if (recv_buf[3] & 0x04) {
				uint8_t signature[16];
				uint8_t * ptr_sig = (uint8_t*)recv_buf[1];
				uint8_t * ptr_code = ptr_sig + 0x20;
				uint64_t code_size = *((uint64_t *)(ptr_sig + 0x10));

				calc_sig(ptr_code, code_size, signature);

				alt_printf("\t- Log: verifying code... ");
				if (comp_sig(ptr_sig, signature) != 1) {
    				alt_printf("FAIL!\n");
					recv_buf[0] = 0x01; // verification failed
					s3k_send(soccaps[1], recv_buf, 0x30, false);
					capman_update();
					break;
				}

				alt_printf("SUCCESS!\n");
			}
#endif

			if ((err = capman_update_pmp(0x30, begin, end, recv_buf[3])) != 0) {
				alt_printf("\t- Log: PMP update failed\n");
				recv_buf[0] = err;
				s3k_send(soccaps[1], recv_buf, 0x30, false);
				break;
			}

			alt_printf("\t- Log: PMP successfully updated\n");

			recv_buf[0] = 0x00;
			s3k_send(soccaps[1], recv_buf, 0x30, false);
			capman_update();
			break;
		default:
			alt_printf("\n[MON]\tMsg from app\n\ttag:%X msg=%X,%X,%X,%X\n", tag, recv_buf[0], recv_buf[1], recv_buf[2], recv_buf[3]);
	}
}
