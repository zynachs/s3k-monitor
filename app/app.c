#include "app.h"
#include "test.h"
#include "trap.h"

#include "s3k.h"
#include "altio.h"
#include "capman.h"
#include "altmem.h"

#include <string.h>

/* config file for testing */
#include "../config.h"

uint8_t pmpcaps[2][8] = { {0}, {0} };
uint8_t soccaps[8] = { 0 }; /* stored as pairs: [0] send + [1] recv, [2] send + [3] recv */

uint64_t buf[4] = {0x00, 0x00, 0x00, 0x00};
uint64_t tag;

static void * ptr = NULL;

void setup(void)
{  
    alt_printf("\n[APP]\tSetup\n");

    /* Setup trap handler */
    alt_printf("\t- trap handler\n");
    s3k_setreg(S3K_REG_TPC, (uint64_t)trap_handler);
    s3k_setreg(S3K_REG_TSP, (uint64_t)trapstack + sizeof(trapstack));

    /* Setup capman */
    alt_printf("\t- caps{\n");
    capman_init();
    capman_getpmp(pmpcaps[0]);
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

    /* Show which test to run */
    alt_printf("\n[APP]\tTESTS: ");
    for (int i = 0; i < 32; i++) {
        if (!(TEST_MASK & (1 << i))) {
            continue;
        }

        alt_printf("0x%X ", i);
    }
    alt_printf("\n");
}

void test(int i) {
    switch (i) {
        /* Test the implementation of the policy W ^ X */
        case 1:
            alt_printf("\tAttempting to write outside process memory...\n");
            test1();       
            break;
        case 2:
            alt_printf("\tAttempting to execute from outside process memory...\n");
            test2();
            break;
        case 3:
            alt_printf("\tAttempting to write to code...\n");
            test3();
            break;
        case 4:
            alt_printf("\tAttempting to execute from data...\n");
            test4();
            break;

        /* Test functionality to change between W and X permissions */
        case 5:
            alt_printf("\tAttempting to write data into allocatable space...\n");
            ptr = test5();
            break;
        case 6:
            alt_printf("\tAttempting to execute code written in an allocated chunk...\n");
            test6(ptr);
            break;
        case 7:
            alt_printf("\tAttempting to execute new code with broken signature...\n");
            test7();
            break;
        
        /* Test miscellaneous functionality */
        case 8:
            alt_printf("\tAttempting to write to rodata...\n");
            test8();
            break;
        case 9:
            alt_printf("\tSimulate that the stack reaches maxsize...\n");
            test9();
            break;
    }
}

void loop(void)
{
    static uint64_t i = 0;

    while (i++ < 32) {

        if (!(TEST_MASK & (1 << i))) {
            continue;
        }

        alt_printf("\n[APP]\tLoop, Running test: #0x%X\n", i);
        test(i);
    }

    /* App has has finished executing */
    buf[0] = -1ull;
	s3k_send(soccaps[0], buf, -1ull, false);
    
    while (true)
        s3k_yield();
}
