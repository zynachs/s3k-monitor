#pragma once

#include <stdint.h>

extern char trapstack[2048]; /* dedicated stack for trap_handler, will probably be place in .bss */

void trap_handler(void) __attribute__((interrupt("machine")));
void th_softreset(void);
void th_hardreset(void);
int th_updatepmp_rw(uint64_t idx, uint64_t rwx);
int th_updatepmp_rx(uint64_t idx, uint64_t sig_addr, uint64_t rwx);
int th_stackexpansion(uint64_t esp);
