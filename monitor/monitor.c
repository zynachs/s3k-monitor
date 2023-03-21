#include <stdbool.h>
#include <string.h>

#include "s3k.h"
#include "uart.h"

#define MONITOR_PID 0
#define APP_PID 1

/* Defines CAP index */
#define CAP_PMP_INIT 0
#define CAP_MEM_MAIN 1
#define CAP_MEM_UART 2
#define CAP_TIME0    3
#define CAP_TIME1    4
#define CAP_TIME2    5
#define CAP_TIME3    6
#define CAP_MON	     7
#define CAP_CHAN     8

void setup_app(void)
{
	/* Set PC */ 
	s3k_msetreg(CAP_MON, APP_PID, S3K_REG_PC, 0x80020000);
	
	/* PMP capabilities are in index 0,1 */
	s3k_msetreg(CAP_MON, APP_PID, S3K_REG_PMP, 0x0100);

	/* Derive memory for APP */
	uint64_t uartaddr = s3k_pmp_napot_addr(0x10000000, 0x10000100);
	uint64_t mainaddr = s3k_pmp_napot_addr(0x80020000, 0x80030000);
	union s3k_cap uartcap = s3k_pmp(uartaddr, S3K_RW);
	union s3k_cap maincap = s3k_pmp(mainaddr, S3K_RWX);
	while (s3k_drvcap(CAP_MEM_UART, 10, uartcap))
		;
	while (s3k_drvcap(CAP_MEM_MAIN, 11, maincap))
		;
	s3k_setreg(S3K_REG_PMP, 0x0b00);

	/* Give capabilities to APP */
	while (s3k_mgivecap(CAP_MON, APP_PID, 11, 0))
		;
	while (s3k_mgivecap(CAP_MON, APP_PID, 10, 1))
		;

	/* Derive time for APP */
	while (s3k_drvcap(CAP_TIME0, 13, s3k_time(0, 0, 4)))
		;
	while (s3k_mgivecap(CAP_MON, APP_PID, 13, 3))
		;
	
}

void setup_monitor(void) 
{
	/* Setup monitor with capabilities to UART memory */
	uint64_t uartaddr = s3k_pmp_napot_addr(0x10000000, 0x10000100);
	union s3k_cap uartcap = s3k_pmp(uartaddr, S3K_RW);
	while (s3k_drvcap(CAP_MEM_UART, 10, uartcap))
		;
	s3k_setreg(S3K_REG_PMP, 0xa00);
}

void setup(void)
{
	/* Delete time on core 2 3 4. */
	s3k_delcap(CAP_TIME1);
	s3k_delcap(CAP_TIME2);
	s3k_delcap(CAP_TIME3);

	/* Setup app and monitor */
	setup_app();
	setup_monitor();

	/* Start app */
	s3k_mresume(CAP_MON, APP_PID);
	s3k_yield();
}

void loop(void)
{
	/* Does nothing, tells the kernel that it is done. */
	s3k_yield();
}
