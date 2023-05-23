#include "../config.h"
#include "altio.h"
#include "base.h"
#include "capman.h"
#include "payload.h"
#include "ring_buffer.h"
#include "s3k.h"

#include <stdbool.h>
#include <string.h>

#define MONITOR_PID 0
#define APP_PID 1

uint8_t pmpcaps[8] = { 0 };
char trapstack[4096];

void traphandler(void) __attribute__((interrupt("user")));

void traphandler(void) 
{
}

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

void setup_app(void) 
{
	/* Copy over binary */
	capman_derive_pmp(0x20, APP_BASE, APP_BASE + 0x10000, S3K_RWX);
	pmpcaps[1] = 0x20;
	capman_setpmp(pmpcaps);
	memcpy((void *)APP_BASE, app_bin, app_bin_len);

	/* Give APP PMP slice */
	capman_mgivecap(APP_PID, 0x20, 0x0);
	/* Give APP memory slice */
	capman_mgivecap(APP_PID, 0x10, 0x10);
	/* Give APP time slice */
	capman_mgivecap(APP_PID, 0x18, 0x18);

	// UART access
	capman_derive_pmp(0x20, (uint64_t)UART_BASE, (uint64_t)UART_BASE + 0x8, S3K_RW);
	capman_mgivecap(MONITOR_PID, 0x20, 0x1);

	/* Set APP PC */
	capman_msetreg(APP_PID, S3K_REG_PC, APP_BASE);
	/* Set APP PMP */
	capman_msetreg(APP_PID, S3K_REG_PMP, 0x0100);
}

void setup(void)
{
	/* Delete time on core 2 3 4. */
	s3k_delcap(4);
	s3k_delcap(5);
	s3k_delcap(6);

	/* Setup trap handler */
	s3k_setreg(S3K_REG_TPC, (uint64_t)traphandler);
	s3k_setreg(S3K_REG_TSP, (uint64_t)trapstack + sizeof(trapstack));

	// Initialize capman
	capman_init();

	// We can now print stuff
	alt_puts("\nboot: Setting up.");
	alt_puts("\ntesting alt_printf");
	alt_puts("------------------");
	alt_printf("plain: hello, world\n");
	alt_printf("char %%c: %c\n", 'A');
	alt_printf("string %%s: %s\n", "hello, world");
	alt_printf("4B hex %%x: 0x%x\n", 0xDEADBEEF);
	alt_printf("8B hex %%X: 0x%X\n", 0xcafedeadbeefull);
	alt_puts("------------------");

	/* Setup memory and time */
	setup_memory_slices();
	setup_time_slices();

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
