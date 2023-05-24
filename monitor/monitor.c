#include "../misc/config.h"
#include "altio.h"
#include "base.h"
#include "capman.h"
#include "payload.h"
#include "ring_buffer.h"
#include "s3k.h"
#include "code-auth.h"
#include "aes128.h"

#include <stdbool.h>
#include <string.h>

#define MONITOR_PID 0
#define APP_PID 1

// Hypothetically provided at distribution of code/application.
uint8_t signature[16] = {0x39, 0x25, 0x84, 0x1d, 0x02, 0xdc, 0x09, 0xfb, 0xdc, 0x11, 0x85, 0x97, 0x19, 0x6a, 0x0b, 0x32};

uint8_t pmpcaps[8] = { 0 };

void setup_memory_slices(void) 
{
	/* App memory */
	capman_derive_mem(0x10, APP_BASE, APP_BASE + 0x10000, S3K_RWX);

	capman_delcap(0x1);
}

void setup_time_slices(void) 
{
	uint64_t hartid = 0;
	/* App time */
	capman_derive_time(0x18, hartid, 0, 32);
}

void setup_monitor(void)
{
	/* Give monitor access to UART memory */
	capman_derive_pmp(0x09, (uint64_t)UART_BASE, (uint64_t)UART_BASE + 0x8, S3K_RW);
	pmpcaps[2] = 0x09;

	capman_setpmp(pmpcaps);
}

void setup_app(void) 
{
	/* Give APP PMP slice */
	capman_mgivecap(APP_PID, 0x20, 0x0);
	/* Give APP memory slice */
	capman_mgivecap(APP_PID, 0x10, 0x10);
	/* Give APP time slice */
	capman_mgivecap(APP_PID, 0x18, 0x18);

	// Create capability to access UART memory, then give it to APP
	capman_derive_pmp(0x20, (uint64_t)UART_BASE, (uint64_t)UART_BASE + 0x8, S3K_RW);
	capman_mgivecap(APP_PID, 0x20, 0x1);

	/* Set APP PC */
	capman_msetreg(APP_PID, S3K_REG_PC, APP_BASE);
	/* Set APP PMP */
	capman_msetreg(APP_PID, S3K_REG_PMP, 0x0100);
}

void load_app()
{
	alt_puts("inside load_app");
	
	/* Give monitor access to APP memory */
	capman_derive_pmp(0x20, APP_BASE, APP_BASE + 0x10000, S3K_RWX);
	pmpcaps[1] = 0x20;
	capman_setpmp(pmpcaps);

	/*Copy app-binary to allocated memory: */
	/*alt_printf("%d", app_bin_len); 6162*/
	memcpy((void *)APP_BASE, app_bin, app_bin_len);
}

void setup(void)
{
	/* Delete time on core 2 3 4. */
	s3k_delcap(4);
	s3k_delcap(5);
	s3k_delcap(6);

	// Initialize capman
	capman_init();

	/* Setup monitor */
	setup_monitor();

	/* Setup memory and time */
	setup_memory_slices();
	setup_time_slices();
	
	/* Load app to allocated memory */
	load_app();

	/* Verify app with CBC-MAC*/
	code_auth((void *)APP_BASE, signature, app_bin_len);

	/* Setup app */
	setup_app();

	s3k_yield();

	/* Start app */
	alt_puts("Resuming app...");
	capman_mresume(APP_PID);
	s3k_yield();
}

void loop(void)
{
	alt_puts("This is monitor loop!");
	/* Does nothing, tells the kernel that it is done. */
	s3k_yield();
}
