#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

int altmem_init(void);
void altmem_dump(void);
void * altmem_alloc(size_t size);
void altmem_free(void * ptr);
bool altmem_stackexpansion(void *ptr);
int altmem_findchunk(void * ptr);
int altmem_isoccupied(int i);
void * altmem_get_start(int i);
uint64_t altmem_get_size(int i);
