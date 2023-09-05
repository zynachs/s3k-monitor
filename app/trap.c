#include "trap.h"
#include "base.h"
#include "s3k.h"
#include "app.h"
#include "altmem.h"
#include "altio.h"
#include "capman.h"

char trapstack[2048]; /* dedicated stack for trap_handler, will probably be place in .bss */

void th_softreset(void)
{
    alt_printf("\t- Action: soft-reset\n");
    s3k_setreg(S3K_REG_EPC, (uint64_t)&loop);
    s3k_setreg(S3K_REG_ESP, (uint64_t)&_stack_top);

    /* App failed to handle an exception */
    buf[0] = 0x01;
	s3k_send(soccaps[0], buf, -1ull, false);
    s3k_yield();

    return;
}

void th_hardreset(void)
{
    alt_printf("\t- Action: hard-reset\n");
    s3k_setreg(S3K_REG_EPC, (uint64_t)&setup);
    s3k_setreg(S3K_REG_ESP, (uint64_t)&_stack_top);

    /* App failed to handle an exception */
    buf[0] = 0x01;
	s3k_send(soccaps[0], buf, -1ull, false);
    s3k_yield();

    return;
}

int th_updatepmp_rw(uint64_t idx, uint64_t rwx) 
{
    buf[0] = 0x02; // instruction to monitor that app wants to update pmp
    buf[1] = 0x00;
    buf[2] = 0x00;
    buf[3] = rwx; // pmp rwx
	s3k_send(soccaps[0], buf, idx, false); // request monitor to update pmp
    s3k_recv(soccaps[1], buf, idx, &tag); // wait for monitor to respond
    capman_update();

    return buf[0];
}

int th_updatepmp_rx(uint64_t idx, uint64_t sig_addr, uint64_t rwx) 
{
    buf[0] = 0x02; // instruction to monitor that app wants to update pmp
    buf[1] = sig_addr;
    buf[2] = 0x00;
    buf[3] = rwx; // pmp rwx
	s3k_send(soccaps[0], buf, idx, false); // request monitor to update pmp
    s3k_recv(soccaps[1], buf, idx, &tag); // wait for monitor to respond
    capman_update();

    return buf[0];
}

int th_stackexpansion(uint64_t esp)
{
    uint64_t i;
    uint64_t v;

    if (altmem_stackexpansion((void*)esp)) {
        i = altmem_findchunk((void*)esp);
        v = (uint64_t)altmem_get_start(i);
        i = capman_find_existing_pmp(v, v + altmem_get_size(i));
        alt_printf("\t- Log: update PMP request sent to monitor\n");
        return th_updatepmp_rw(i, S3K_RW);
    }
    return 1;
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

    alt_printf("\n[APP]\ttrap handler\n");

    switch (ecause) {
        case 1:
            alt_printf("\t- ecause: 1 \"Instruction access fault\"\n\t- eval: 0x%X\n\t- epc: 0x%X\n", eval, epc);

            /* Check if code is within process memory */
            if (epc < (uint64_t)&_start || epc >= (uint64_t)&_stack) {
                alt_printf("\t- Log: instruction outside of process memory\n");
                th_softreset();
                return;
            }
            
            alt_printf("\t- Log: instruction within process memory\n");

            /* Check if code is in an allocatable chunk */
            if ((itemp = altmem_findchunk((void*)epc)) == -1) {
                alt_printf("\t- Log: instruction is not within an allocatable chunk\n", epc);
                th_softreset();
                return;
            }
            
            alt_printf("\t- Log: instruction is within an allocatable chunk\n");

            /* Check if chunk is allocated */
            if (altmem_isoccupied(itemp) <= 0) {
                alt_printf("\t- Log: instruction is not within an allocated chunk\n", epc);
                th_softreset();
                return;
            }

            alt_printf("\t- Log: instruction is within an allocated chunk\n");

            vtemp = (uint64_t)altmem_get_start(itemp); // get start address of relevant chunk
            itemp = capman_find_existing_pmp(vtemp, vtemp + altmem_get_size(itemp)); // get capability index for the existing pmp

            /* Update PMP to RX, let monitor allow or deny */
            alt_printf("\t- Log: update PMP request sent to monitor\n");
            etemp = th_updatepmp_rx(itemp, vtemp, 0x5);

	        if (etemp != 0x00) {
	            alt_printf("\n[APP]\t- Log: PMP unsuccesfully updated; ERROR: %X\n", etemp);
                th_softreset();
                return;
            }

            alt_printf("\n[APP]\t- Log: PMP successfully updated\n");
	        alt_printf("\t- Action: set chunk permissions to RX\n");
	        break;
        case 5:
            alt_printf("\t- ecause: 5 \"Load access fault\"\n\t- eval: 0x%X\n\t- epc: 0x%X\n", eval, epc);
            th_softreset();
            return;
        case 7:
            alt_printf("\t- ecause: 7 \"Store/AMO access fault\"\n\t- eval: 0x%X\n\t- epc: 0x%X\n", eval, epc);
            
            /* Stack pointer has exceeded its limit, attempt to expand stack. */
            if (esp <= (uint64_t)&_stack) {
                alt_printf("\t- Log: stack maxsize exceeded\n");
                etemp = th_stackexpansion(esp);
                if (etemp != 0) {
                    alt_printf("\t- Log: stack unsuccesfully expanded; ERROR: %X\n", etemp);
                    th_softreset();
                    return;
                }
                alt_printf("\t- Log: stack succesfully expanded\n");
                alt_printf("\t- Action: expand stack\n");
                break;
            } 

	        /* Check if memory is in an allocatable chunk */
            if ((itemp = altmem_findchunk((void*)eval)) == -1) {
                alt_printf("\t- Log: 0x%X does not match an allocatable chunk\n", eval);
                th_softreset();
                return;
            } 
            
            alt_printf("\t- Log: address is within an allocatable chunk\n");

            /* Check if memory is in an allocated chunk */ 
            if (altmem_isoccupied(itemp) == -1) {
                alt_printf("\t- Log: chunk is not in an allocated chunk\n");
                th_softreset();
                return;
            }
            
            alt_printf("\t- Log: chunk is allocated\n");

            vtemp = (uint64_t)altmem_get_start(itemp);
            itemp = capman_find_existing_pmp(vtemp, vtemp + altmem_get_size(itemp));

            /* Request to update PMP */
            alt_printf("\t- Log: update PMP request sent to monitor\n");
            etemp = th_updatepmp_rw(itemp, S3K_RW);

            if (etemp != 0x00) {
                alt_printf("\n[APP]\t- Log: PMP unsuccesfully updated; ERROR: %X\n", etemp);
                th_softreset();
                return;
            }
            
            alt_printf("\n[APP]\t- Log: PMP successfully updated\n");
            alt_printf("\t- Action: set chunk permissions to RW\n");
            break;
        default:
            alt_printf("\t- ecause: %X\n\t- eval %X\n\t- epc: %X\n", ecause, eval, epc);
            th_hardreset();
    }

    /* App has successfully handled an exception */
    buf[0] = 0x00;
	s3k_send(soccaps[0], buf, -1ull, false);

    s3k_yield(); /* relinquishes remaining time to allow for monitor to execute before app proceedes. */
    return;
}
