#include <string.h>

#include "uart.h"
#include "s3k.h"
#include "printf.h"
#include "trap.h"

extern const volatile uint64_t _trapstack_top;

void setup(void);
void loop(void);

void trap_handler(void){
    printf("This is the trap handler! Returning to loop.\nSP at exception: %d\n", s3k_getreg(S3K_REG_ESP));
    s3k_setreg(S3K_REG_EPC, (uint64_t)loop);
    s3k_setreg(S3K_REG_ESP, (uint64_t)0x80030000);
}

void setup(void)
{  
    /* Setup trap handler */
    s3k_setreg(S3K_REG_TPC, (uint64_t)trap_handler);
    s3k_setreg(S3K_REG_TSP, (uint64_t)&_trapstack_top);

    #if defined __TEST1 || defined __TEST2  
    /* Compiled malicious machine code, created using "make genpayload" */
    static char malicious_string[272] = {
        0x01,0x11,0x22,0xec,0x00,0x10,0xb7,0x07,0x00,0x10,0x23,0x34,0xf4,0xfe,0x83,0x37,
        0x84,0xfe,0x13,0x07,0x50,0x04,0x23,0x80,0xe7,0x00,0x83,0x37,0x84,0xfe,0x13,0x07,
        0x80,0x07,0x23,0x80,0xe7,0x00,0x83,0x37,0x84,0xfe,0x13,0x07,0x50,0x06,0x23,0x80,
        0xe7,0x00,0x83,0x37,0x84,0xfe,0x13,0x07,0x30,0x06,0x23,0x80,0xe7,0x00,0x83,0x37,
        0x84,0xfe,0x13,0x07,0x50,0x07,0x23,0x80,0xe7,0x00,0x83,0x37,0x84,0xfe,0x13,0x07,
        0x40,0x07,0x23,0x80,0xe7,0x00,0x83,0x37,0x84,0xfe,0x13,0x07,0x90,0x06,0x23,0x80,
        0xe7,0x00,0x83,0x37,0x84,0xfe,0x13,0x07,0xe0,0x06,0x23,0x80,0xe7,0x00,0x83,0x37,
        0x84,0xfe,0x13,0x07,0x70,0x06,0x23,0x80,0xe7,0x00,0x83,0x37,0x84,0xfe,0x13,0x07,
        0x00,0x02,0x23,0x80,0xe7,0x00,0x83,0x37,0x84,0xfe,0x13,0x07,0x60,0x06,0x23,0x80,
        0xe7,0x00,0x83,0x37,0x84,0xfe,0x13,0x07,0x20,0x07,0x23,0x80,0xe7,0x00,0x83,0x37,
        0x84,0xfe,0x13,0x07,0xf0,0x06,0x23,0x80,0xe7,0x00,0x83,0x37,0x84,0xfe,0x13,0x07,
        0xd0,0x06,0x23,0x80,0xe7,0x00,0x83,0x37,0x84,0xfe,0x13,0x07,0x00,0x02,0x23,0x80,
        0xe7,0x00,0x83,0x37,0x84,0xfe,0x13,0x07,0x40,0x06,0x23,0x80,0xe7,0x00,0x83,0x37,
        0x84,0xfe,0x13,0x07,0x10,0x06,0x23,0x80,0xe7,0x00,0x83,0x37,0x84,0xfe,0x13,0x07,
        0x40,0x07,0x23,0x80,0xe7,0x00,0x83,0x37,0x84,0xfe,0x13,0x07,0x10,0x06,0x23,0x80,
        0xe7,0x00,0x83,0x37,0x84,0xfe,0x13,0x07,0x10,0x02,0x23,0x80,0xe7,0x00,0x83,0x37,
        0x84,0xfe,0x29,0x47,0x23,0x80,0xe7,0x00,0x01,0x00,0x62,0x64,0x05,0x61,0x82,0x80
    };
    #endif

    #if defined __TEST1 || defined __TEST3 
    /* Create a pointer to a function */
	typedef void (*func_t)(void);
    #endif

    #ifdef DEBUG
    /* DEBUG: prints active PMP registers and the capabilities */ 
	uart_puts("=== APP DEBUG INFORMATION ===");
    printf("REG_PMP: %lx\n", s3k_getreg(S3K_REG_PMP));
    for (uint64_t i = 0; i < 16; i++) 
    {
        printf("CAP %d: %lx\n", i, s3k_getcap(i));
    }
	uart_puts("=== END OF DEBUG INFORMATION ===");
    #endif

    #ifdef __TEST1
    /* Call malicious_string as a function. Expected outcome: EXCEPTION, mcause=1 (instruction access fault) */ 
    uart_puts("App attempting to execute from data...");
	((func_t)malicious_string)();
    #endif

    #ifdef __TEST2
    /* Copies malicious_string to _rodata. Expected outcome: EXCEPTION, mcause=7 (store/AMO access fault) */
    uart_puts("App attempting to write to rodata...");
    void * ptr = (void*)0x80021800;
    memcpy(ptr, malicious_string, 272);
    #endif

    #ifdef __TEST3 
    /* Calls code in the monitor as a function. Expected outcome: EXCEPTION, mcause=1 (instruction access fault) */
    void * ptr = (void*)0x80010302; /* address to monitor loop function */
    uart_puts("App attempting to execute from monitors space...");
	((func_t)ptr)();
    #endif

    s3k_yield();
}

void loop(void)
{
    uart_puts("This is app loop! test");
    s3k_yield();
}
