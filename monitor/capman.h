#pragma once
#include "s3k.h"

#include <stdbool.h>
#include <stdint.h>

void capman_init(void);
union s3k_cap capman_get(uint64_t idx);
bool capman_move(uint64_t src, uint64_t dst);
bool capman_delcap(uint64_t idx);
void capman_update(void);
void capman_dump(union s3k_cap);
void capman_dump_all(void);
int capman_find_free(void);
void capman_getpmp(uint8_t pmp[8]);
void capman_setpmp(uint8_t pmp[8]);
bool capman_derive_mem(int idx, uint64_t begin, uint64_t end, uint64_t rwx);
bool capman_derive_time(int idx, uint64_t hartid, uint64_t begin, uint64_t end);
bool capman_derive_pmp(int idx, uint64_t begin, uint64_t end, uint64_t rwx);
bool capman_mresume(uint64_t pid);
bool capman_msuspend(uint64_t pid);
bool capman_mgivecap(uint64_t pid, uint64_t src, uint64_t dest);
bool capman_mtakecap(uint64_t pid, uint64_t src, uint64_t dest);
bool capman_msetreg(uint64_t pid, uint64_t reg, uint64_t val);
bool capman_mgetreg(uint64_t pid, uint64_t reg, uint64_t *val);
