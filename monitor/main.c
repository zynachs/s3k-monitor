#include <stdbool.h>
#include <string.h>

#include "s3k.h"
#include "uart.h"

#define NOINIT	      __attribute__((section(".noinit")))
#define ALIGNED(x)    __attribute__((aligned(x)))
#define ARRAY_SIZE(x) ((sizeof(x)) / (sizeof(x[0])))

#define MEMSIZE 8192

#define MONITOR_PID 0

#define CAP_PMP_INIT 0
#define CAP_MEM_MAIN 1
#define CAP_MEM_UART 2
#define CAP_TIME0    3
#define CAP_TIME1    4
#define CAP_TIME2    5
#define CAP_TIME3    6
#define CAP_MON	     7
#define CAP_CHAN     8

void setup(void)
{
	/* Delete time on core 2 3 4. */
	s3k_delcap(CAP_TIME1);
	s3k_delcap(CAP_TIME2);
	s3k_delcap(CAP_TIME3);

	uint64_t uartaddr = pmp_napot_addr(0x10000000, 0x10001000);
	union s3k_cap uartcap = s3k_pmp(uartaddr, S3K_RW);
	while (s3k_drvcap(CAP_MEM_UART, 10, uartcap))
		;

	s3k_setreg(S3K_REG_PMP, 0xa00);
	s3k_yield();
}

void loop(void)
{
	uart_puts("hello world!");
}
