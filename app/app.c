#include <string.h>

#include "altio.h"
#include "s3k.h"
#include "capman.h"
#include "altmem.h"

extern const void * _start; /* base address of app, defined in start.S */
extern const void * _stack_top; /* address of stack top, defined in start.S */
extern const void * _stack; /* address of bottom of stack, defined in start.S */
extern const void * _heap;
extern const void * _rodata;

uint8_t pmpcaps[8] = { 0 };
uint8_t soccaps[8] = { 0 }; /* stored as pairs: [0] send + [1] recv, [2] send + [3] recv */

uint64_t buf[4] = {0x00, 0x00, 0x00, 0x00};
uint64_t tag;

char trapstack[2048]; /* dedicated stack for trap_handler, will probably be place in .bss */

void trap_handler(void) __attribute__((interrupt("user")));
void th_softreset(void);
void setup();
void loop();

/* Compiled malicious machine code, created using "make genpayload" */
static const uint8_t malicious_string[272] = {
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
}; /* Output: "Executing from data!" */

typedef void (*func_t)(void);

void th_softreset(void)
{
    alt_printf("\t- Action: soft-reset\n");
    s3k_setreg(S3K_REG_EPC, (uint64_t)&loop);
    s3k_setreg(S3K_REG_ESP, (uint64_t)&_stack_top);
    return;
}

void trap_handler(void)
{
    /* Store the error code and value of the error */
    uint64_t epc = s3k_getreg(S3K_REG_EPC);
    uint64_t ecause = s3k_getreg(S3K_REG_ECAUSE);
    uint64_t eval = s3k_getreg(S3K_REG_EVAL);
    uint64_t esp = s3k_getreg(S3K_REG_ESP);
    uint64_t itemp = 0;
    uint64_t vtemp = 0;
    uint64_t etemp = 0;

    switch (ecause) {
        case 1:
            alt_printf("\t- ecause: 1 \"Instruction access fault\"\n\t- eval: 0x%X\n\t- epc: 0x%X\n", eval, epc);

            /* Check if code is within process memory */
            if (epc < (uint64_t)&_start || epc >= (uint64_t)&_stack) {
                alt_printf("\t- Log: instruction outside of process memory\n");
                th_softreset();
                break;
            }
            
            alt_printf("\t- Log: instruction within process memory\n");

            /* Check if code is in an allocatable chunk */
            if ((itemp = altmem_findchunk((void*)epc)) == -1) {
                alt_printf("\t- Log: instruction is not within an allocatable chunk\n", epc);
                th_softreset();
                break;
            }
            
            alt_printf("\t- Log: instruction is within an allocatable chunk\n");

            /* Check if chunk is allocated */
            if (altmem_isoccupied(itemp) <= 0) {
                alt_printf("\t- Log: instruction is not within an allocated chunk\n", epc);
                th_softreset();
                break;
            }

            alt_printf("\t- Log: instruction is within an allocated chunk\n");

            /* Update PMP to RX, let monitor allow or deny */
            // IPC with monitor, await response
            buf[0] = 0x02;
	        s3k_send(soccaps[0], buf, -1ull, false);
            s3k_recv(soccaps[1], buf, -1ull, &tag);

	        if ((etemp = buf[0]) != 0x00) {
	            alt_printf("[APP]\t- Log: PMP unsuccesfully updated; ERROR: %X\n", etemp);
                th_softreset();
                break;
            }

            alt_printf("[APP]\t- Log: PMP successfully updated\n");
	        alt_printf("\t- Action: set chunk permissions to RX\n");
	        break;
        case 5:
            alt_printf("\t- ecause: 5 \"Load access fault\"\n\t- eval: 0x%X\n\t- epc: 0x%X\n", eval, epc);
            alt_printf("\t- Action: soft-reset\n");
            s3k_setreg(S3K_REG_EPC, (uint64_t)&loop);
            s3k_setreg(S3K_REG_ESP, (uint64_t)&_stack_top);
            break;
        case 7:
            alt_printf("\t- ecause: 7 \"Store/AMO access fault\"\n\t- eval: 0x%X\n\t- epc: 0x%X\n", eval, epc);

            /* Stack pointer has exceeded its limit, attempt to expand stack. */
            if (esp <= (uint64_t)&_stack) {
                alt_printf("\t- Log: stack maxsize exceeded\n");
                if (altmem_stackexpansion((void*)esp)) {
                    itemp = altmem_findchunk((void*)esp);
                    vtemp = (uint64_t)altmem_get_start(itemp);
                    if ((etemp = capman_update_pmp(0x20, vtemp, vtemp + altmem_get_size(itemp), S3K_RW)) == 0) {
                        alt_printf("\t- Log: stack succesfully expanded\n");
                        alt_printf("\t- Action: expand stack\n");
                        break;
                    }
                    alt_printf("\t- Log: stack unsuccesfully expanded; ERROR: %X\n", etemp);
                } else {
                    alt_printf("\t- Log: stack unsuccesfully expanded; ERROR: adjacent chunk not available\n", vtemp);
                }
            } 
            else {
	        /* Check if memory is allocated, update pmp with write permissions. */
	            if ((itemp = altmem_findchunk((void*)eval)) == -1) {
                    alt_printf("\t- Log: 0x%X does not match an allocatable chunk\n", eval);
	            } else if (altmem_isoccupied(itemp) != -1) {
                    alt_printf("\t- Log: chunk is allocated\n");
	                vtemp = (uint64_t)altmem_get_start(itemp);
	                if ((etemp = capman_update_pmp(0x20, vtemp, vtemp + altmem_get_size(itemp), S3K_RW)) == 0) {
	                    alt_printf("\t- Log: PMP successfully updated\n");
	                    alt_printf("\t- Action: set chunk permissions to RW\n");
	                    break;
	                }
	                alt_printf("\t- Log: PMP unsuccesfully updated; ERROR: %X\n", etemp);
	            } else {
	                alt_printf("\t- Log: PMP unsuccesfully updated; ERROR: requested chunk is not allocated\n", vtemp);
                }
            }

            // Default   
            alt_printf("\t- Action: soft-reset\n");
            s3k_setreg(S3K_REG_EPC, (uint64_t)&loop);
            s3k_setreg(S3K_REG_ESP, (uint64_t)&_stack_top);
            break;
        default:
            alt_printf("\t- ecause: %X\n\t- eval %X\n\t- epc: %X\n", ecause, eval, epc);
            alt_printf("\t- Action: hard-reset\n");
            s3k_setreg(S3K_REG_EPC, (uint64_t)&_start);
            s3k_setreg(S3K_REG_ESP, (uint64_t)&_stack_top);
    }

    /* App has handled an exception */
    buf[0] = 0x01;
	s3k_send(soccaps[0], buf, -1ull, false);

    s3k_yield(); /* relinquishes remaining time to allow for monitor to execute before app proceedes. */
    return;
}

void test1(void) 
{
    /* Call malicious_string as a function. Expected outcome: EXCEPTION, mcause=1 (instruction access fault) */ 
	((func_t)malicious_string)();
    return;
}

void test2(void) 
{
    static uint8_t arbitrary_data[] = {0xDE,0xAD,0xBE,0xEF};

    /* Copies arbitrary data to _rodata. Expected outcome: EXCEPTION, mcause=7 (store/AMO access fault) */
    void * ptr = &_rodata;
    memcpy(ptr, arbitrary_data, 4);

    return;
}

void test3(void) 
{
	typedef void (*func_t)(void);

    /* Calls code in the monitor as a function. Expected outcome: EXCEPTION, mcause=1 (instruction access fault) */
    void * ptr = (void*)0x80010100; /* address to code in monitor. */
	((func_t)ptr)();

    return;
}

void test4(void)
{
    static uint8_t arbitrary_data[] = {0xDE,0xAD,0xBE,0xEF}; 
    
    /* Writes data to heap. Expected outcome: EXCEPTION, mcause=7 (Store/AMO access fault).
       Should be able to recover by creating a new PMP with access. */
    int size = sizeof(arbitrary_data) / sizeof(arbitrary_data[0]);
    void * ptr = altmem_alloc(size);
    memcpy(ptr, arbitrary_data, size);
    altmem_free(ptr);
    return;
}

void test5(void)
{
    /* Sets stack pointer to _stack and attempts to push data to stack. Expected outcome: EXCEPTION, mcause=7 (Store/AMO access fault).
       Should be able to recover by creating a new pmp with access behind the scenes. If there is no more space, kill and restart process. */
	__asm__ volatile("addi sp,sp,-1024");
	__asm__ volatile("addi sp,sp,-1024");
	__asm__ volatile("addi sp,sp,-1024");
	__asm__ volatile("addi sp,sp,-1024");
	__asm__ volatile("li t0,0xDEADBEEF");
	__asm__ volatile("addi sp,sp,-32");
	__asm__ volatile("sd t0,0(sp)");
	__asm__ volatile("addi sp,sp,32");
	__asm__ volatile("addi sp,sp,1024");
	__asm__ volatile("addi sp,sp,1024");
	__asm__ volatile("addi sp,sp,1024");
	__asm__ volatile("addi sp,sp,1024");
    return;
}

void test6(void)
{
    /* Reads outside of process memory. Expected outcome: EXCEPTION, mcause=5 (Load access fault). Should not be able to recover. */
    uint8_t arbitrary_buffer[4];
    void * ptr = (void*)&_stack_top;
    memcpy(arbitrary_buffer, ptr, 4);
    return;
}

void test7(void)
{
    /* Attemps to execute code written in test4. Expected outcome: EXCEPTION, mcause=1 (Instruction access fault). Should be able to recover by asking monitor to verify code. */
    int size = sizeof(malicious_string) / sizeof(malicious_string[0]);
    void * ptr = altmem_alloc(size);
    memcpy(ptr, malicious_string, size);
    ((func_t)ptr)();
    altmem_free(ptr);
    return;
}

void setup(void)
{  
    alt_printf("[APP]\tSetup\n");

    /* Setup trap handler */
    alt_printf("\t- trap handler\n");
    s3k_setreg(S3K_REG_TPC, (uint64_t)trap_handler);
    s3k_setreg(S3K_REG_TSP, (uint64_t)trapstack + sizeof(trapstack));

    /* Setup capman */
    alt_printf("\t- caps{\n");
    capman_init();
    capman_getpmp(pmpcaps);
    capman_getsoc(soccaps);
    capman_dump_all();
	alt_printf("}\n");

    /* Setup altmem */
    alt_printf("\t- mem{\n");
    altmem_init();
    altmem_dump();
	alt_printf("}\n");

    /* Check IPC */
    buf[0] = -1ull;
    alt_printf("\t- ipc, sending 0x%X to monitor\n", buf[0]);
    s3k_send(soccaps[0], buf, -1ull, false);

    s3k_yield();
}

void loop(void)
{
    static uint64_t iloop = 0;

    alt_printf("[APP]\tLoop #0x%X\n", iloop);

    switch (iloop++) {
        case 0:
            alt_printf("\t*Nothing*\n");
            break;
        case 1:
            alt_printf("\tAttempting execution from data...\n");
            test1();
            break;
        case 2:
            alt_printf("\tAttempting to write to rodata...\n");
            test2();
            break;
        case 3:
            alt_printf("\tAttempt to execute from monitor space...\n");
            test3();
            break;
        case 4:
            alt_printf("\tAttempt to write data into unallocated space...\n");
            test4();
            break;
        case 5:
            alt_printf("\tSimulate that the stack reaches maxsize...\n");
            test5();
            break;
        case 6:
            alt_printf("\tAttempts to read outside of process memory...\n");
            test6();
            break;
        case 7:
            alt_printf("\tAttempt to execute code written in an allocated chunk...\n");
            test7();
            break;
        default:
            alt_printf("\t*Restarting*\n");
            s3k_yield();
            s3k_setreg(S3K_REG_PC, (uint64_t)&_start);
    }

    /* No msg from app */
    buf[0] = 0x00;
	s3k_send(soccaps[0], buf, -1ull, false);
    
    s3k_yield();
    return;
}
