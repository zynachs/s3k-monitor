#include "../misc/config.h"
#include "altio.h"
#include "base.h"
#include "capman.h"
#include "payload.h"
#include "s3k.h"
#include "../inc/aes128.h"
#include "../inc/code-auth.h"

#include <stdbool.h>
#include <string.h>

#define MONITOR_PID 0
#define APP_PID 1

#define signature_size 16 
#define header_size 512ull

#define SOCKET_PORT1 0xa
#define SOCKET_PORT2 0xb

struct memory_section {
	char section[8];
	uint8_t rwx;
	uint64_t size;
};


uint8_t pmpcaps[8] = { 0 };
uint8_t soccaps[8] = { 0 };

uint64_t recv_buf[4] = { 0x0, 0x0, 0x0, 0x0 };
uint64_t tag;

void setup_memory_slices(void) 
{
	/* App memory */
	capman_derive_mem(0x10, APP_BASE, APP_BASE + APP_SIZE, S3K_RWX);

	/* Delete access to remaining memory */
	capman_delcap(0x1);
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

	capman_setpmp(pmpcaps);
}

void setup_app(void) 
{
	/* Keep track of PMP capabilities created */
	uint8_t npmp = 0; // number of PMP capabilities created
	uint64_t ipmp = 0x0000000000000000; // byte mask of indices to PMP capabilities

	/* An array of values of the size of memory sections. Could be improved by being loaded dynamically. */
	struct memory_section memory_sections[] = {
		{"text", S3K_RX, CHUNK_SIZE*2}, /* NOTE: 8K */
		{"data", S3K_RW, CHUNK_SIZE},
		{"rodata", S3K_R, CHUNK_SIZE},
		{"bss", S3K_RW, CHUNK_SIZE},
		{"heap0", S3K_R, CHUNK_SIZE}, /* placed directly after .bss*/
		{"heap1", S3K_R, CHUNK_SIZE}, /* Not used, specified by CHUNK_MASK in base.h */
		{"heap2", S3K_R, CHUNK_SIZE}, /* Not used, specified by CHUNK_MASK in base.h */ 
		{"heap3", S3K_R, CHUNK_SIZE}, /* Not used, specified by CHUNK_MASK in base.h */
		{"heap4", S3K_R, CHUNK_SIZE}, /* Not used, specified by CHUNK_MASK in base.h */
		{"heap5", S3K_R, CHUNK_SIZE}, /* Not used, specified by CHUNK_MASK in base.h */
		{"heap6", S3K_R, CHUNK_SIZE}, /* Not used, specified by CHUNK_MASK in base.h */
		{"heap7", S3K_R, CHUNK_SIZE}, /* Not used, specified by CHUNK_MASK in base.h */
		{"heap8", S3K_R, CHUNK_SIZE}, /* Not used, specified by CHUNK_MASK in base.h */
		{"heap9", S3K_R, CHUNK_SIZE}, /* placed directly before .stack*/
		{"stack", S3K_RW, CHUNK_SIZE}
	};

	/* Derive PMP for each section and give to APP */
	uint64_t temp_counter = APP_BASE;
	for (uint8_t j = 0; j < (sizeof(memory_sections) / sizeof(memory_sections[0])) || temp_counter < app_bin_len; j++) {
		/* If chunk is not set to be used do not create pmp for it */
		if (!(CHUNK_MASK & (1 << j))) {
			temp_counter += memory_sections[j].size;
			continue;
		}
		capman_derive_pmp(0x20, temp_counter, temp_counter + memory_sections[j].size, memory_sections[j].rwx);
		temp_counter += memory_sections[j].size;
		capman_mgivecap(APP_PID, 0x20, npmp++);
	} 

	// Create capability to access UART memory, then give it to APP
	capman_derive_pmp(0x20, (uint64_t)UART_BASE, (uint64_t)UART_BASE + 0x100, S3K_RW);
	capman_mgivecap(APP_PID, 0x20, npmp++);
	
	/* Give APP memory slice but only RW */
	capman_derive_mem(0x11, APP_BASE, APP_BASE + APP_SIZE, S3K_RW);
	capman_mgivecap(APP_PID, 0x11, 0x10);

	/* Give APP time slice */
	capman_mgivecap(APP_PID, 0x18, 0x18);

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
	/* Give monitor access to APP memory */
	capman_derive_pmp(0x20, APP_BASE, APP_BASE + APP_SIZE, S3K_RW);
	pmpcaps[1] = 0x20;
	capman_setpmp(pmpcaps);

	/* Copy app-binary to allocated memory: */
	memcpy((void *)APP_BASE, app_bin, app_bin_len);
  	
  /* Revoke access to app memory */
	pmpcaps[1] = 0x0;
	capman_setpmp(pmpcaps);
	capman_delcap(0x20);
}

void setup(void)
{

	/* Delete time on core 2 3 4. */
	s3k_delcap(4);
	s3k_delcap(5);
	s3k_delcap(6);

	/* Initialize capman */
	capman_init();

	/* Setup monitor */
	setup_monitor();
	alt_printf("[MON]\tSetup\n");

	/* Setup memory and time */
	alt_printf("\t- memory\n\t- time\n");
	setup_memory_slices();
	setup_time_slices();

  /* Load app to allocated memory */
	load_app();
  
  // Holder for calculating MAC signature
	uint8_t signature[16];

	/* Calculate mac of app and store in signature */
	uint8_t *ptr_app = (void *)(APP_BASE + header_size);
	calc_sig(ptr_app, app_bin_len - header_size, signature);
  
  /* Authentication, compares provided signature with calculated signature and setup app if successfull */
	if (comp_sig((void *)APP_BASE, signature) != 1){
    alt_puts("Authentication failed");
		return;
  }
  
	/* Setup app */
	alt_printf("\t- app\n");
	setup_app();

	/* Setup IPC */
	alt_printf("\t- ipc\n");
	setup_ipc();
	capman_getsoc(soccaps);

	alt_printf("\t- caps{\n");
	capman_dump_all();
	alt_printf("}\n");

	s3k_yield();

	/* Start app */
	alt_printf("[MON]\tResuming app...\n");
	capman_mresume(APP_PID);

	/* Wait for app to respond */
	s3k_recv(soccaps[0], recv_buf, -1ull, &tag);
	recv_buf[0] == -1ull ? alt_printf("[MON]\tApp setup success! 0x%X\n", recv_buf[0]) : alt_printf("[MON]\tApp setup failed! 0x%X\n", recv_buf[0]);
}

void loop(void)
{
	static uint8_t iloop = 0;
	
	alt_printf("[MON]\tLoop #0x%X\n", iloop++);

	/* Yields until msg read from buffer */	
	s3k_recv(soccaps[0], recv_buf, -1ull, &tag);

	switch (recv_buf[0]) {
		case 0x00:
			alt_printf("[MON]\tNo msg from app\n");
			break;
		case 0x02:
			alt_printf("[MON]\tApp requests to update PMP from RW to RX\n");
			/* Verify code */
			recv_buf[0] = 0x01;
			s3k_send(soccaps[1], recv_buf, -1ull, false);
			break;
		case 0x01:
			alt_printf("[MON]\tApp has handled an exception\n");
			break;
		default:
			alt_printf("[MON]\tMsg from app\n\ttag:%X msg=%X,%X,%X,%X\n", tag, recv_buf[0], recv_buf[1], recv_buf[2], recv_buf[3]);
	}
}
