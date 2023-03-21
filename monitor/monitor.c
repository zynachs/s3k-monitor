#include <stdbool.h>
#include <string.h>

#include "s3k.h"
#include "uart.h"
#include "printf.h"

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
#define CAP_PMP_UART 9

void setup_app(void)
{
	/* Set PC */ 
	s3k_msetreg(CAP_MON, APP_PID, S3K_REG_PC, 0x80020000);
	
	/* PMP capabilities are in index 0,1,2,3,4 */
	s3k_msetreg(CAP_MON, APP_PID, S3K_REG_PMP, 0x50403020100);

	/* Create napot addresses */
	uint64_t uartaddr = s3k_pmp_napot_addr(0x10000000, 0x10000100);
	uint64_t textaddr = s3k_pmp_napot_addr(0x80020000, 0x80021000);
	uint64_t dataaddr = s3k_pmp_napot_addr(0x80021000, 0x80021800);
	uint64_t rodataaddr = s3k_pmp_napot_addr(0x80021800, 0x80022000);
	uint64_t bssaddr = s3k_pmp_napot_addr(0x80022000, 0x80023000);
	uint64_t stackaddr = s3k_pmp_napot_addr(0x8002f800, 0x80030000);

	/* Check values of napot addresses */
	printf("uartaddr = 0x%x\n", uartaddr);
	printf("textaddr = 0x%x\n", textaddr);
	printf("dataaddr = 0x%x\n", dataaddr);
	printf("rodataaddr = 0x%x\n", rodataaddr);
	printf("bssaddr = 0x%x\n", bssaddr);
	printf("stackaddr = 0x%x\n", stackaddr);

	/* Derive PMP for APP */
	union s3k_cap uartcap = s3k_pmp(uartaddr, S3K_RW);
	union s3k_cap textcap = s3k_pmp(textaddr, S3K_RX);
	union s3k_cap datacap = s3k_pmp(dataaddr, S3K_RW);
	union s3k_cap rodatacap = s3k_pmp(rodataaddr, S3K_R);
	union s3k_cap bsscap = s3k_pmp(bssaddr, S3K_RW);
	union s3k_cap stackcap = s3k_pmp(stackaddr, S3K_RW);
	while (s3k_drvcap(CAP_MEM_UART, 10, uartcap))
		;
	while (s3k_drvcap(CAP_MEM_MAIN, 11, textcap))
		;
	while (s3k_drvcap(CAP_MEM_MAIN, 12, datacap))
		;
	while (s3k_drvcap(CAP_MEM_MAIN, 13, rodatacap))
		;
	while (s3k_drvcap(CAP_MEM_MAIN, 14, bsscap))
		;
	while (s3k_drvcap(CAP_MEM_MAIN, 15, stackcap))
		;

	/* Give capabilities to APP */
	while (s3k_mgivecap(CAP_MON, APP_PID, 10, 0))
		;
	while (s3k_mgivecap(CAP_MON, APP_PID, 11, 1))
		;
	while (s3k_mgivecap(CAP_MON, APP_PID, 12, 2))
		;
	while (s3k_mgivecap(CAP_MON, APP_PID, 13, 3))
		;
	while (s3k_mgivecap(CAP_MON, APP_PID, 14, 4))
		;
	while (s3k_mgivecap(CAP_MON, APP_PID, 15, 5))
		;

	/* Derive time for APP */
	while (s3k_drvcap(CAP_TIME0, 16, s3k_time(0, 0, 4)))
		;

	/* Give time to APP */
	while (s3k_mgivecap(CAP_MON, APP_PID, 16, 6))
		;
}

void setup_monitor(void) 
{
	/* Setup monitor with capabilities to UART memory */
	uint64_t uartaddr = s3k_pmp_napot_addr(0x10000000, 0x10000100);
	union s3k_cap uartcap = s3k_pmp(uartaddr, S3K_RW);
	while (s3k_drvcap(CAP_MEM_UART, CAP_PMP_UART, uartcap))
		;
	s3k_setreg(S3K_REG_PMP, 0x0900);
}

void setup(void)
{
	/* Delete time on core 2 3 4. */
	s3k_delcap(CAP_TIME1);
	s3k_delcap(CAP_TIME2);
	s3k_delcap(CAP_TIME3);

	/* Setup app and monitor */
	setup_monitor();
	setup_app();

	/* Test to execute malicious_string in app .data section, expected outcome: mcause = 1
	typedef void (*func_t)(void);
    void * ptr = (void*)0x80021000;
	((func_t)ptr)();
	*/

	/* Start app */
	uart_puts("Resuming app...");
	s3k_mresume(CAP_MON, APP_PID);
	s3k_yield();
}

void loop(void)
{
	uart_puts("This is monitor loop!");
	/* Does nothing, tells the kernel that it is done. */
	s3k_yield();
}
