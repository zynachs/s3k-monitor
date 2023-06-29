#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "base.h"
#include "altio.h"

int altmem_init(void);
void altmem_dump(void);
void * altmem_alloc(size_t size);
void altmem_free(void * ptr);
bool altmem_stackexpansion(void *ptr);
int altmem_findchunk(void * ptr);
int altmem_isoccupied(int i);
void * altmem_get_start(int i);
uint64_t altmem_get_size(int i);

struct chunk {
    void * start;
    uint64_t size;
    bool occupied;
};

extern const void * _heap;
extern const void * _stack;
uint64_t nchunks = 0;
struct chunk chunks[APP_SIZE / CHUNK_SIZE];

int altmem_init(void)
{
    uint64_t temp_counter = (uint64_t)&_heap;
    uint64_t i = 0;
    for (; temp_counter <= ((uint64_t)&_stack - CHUNK_SIZE); i++) {
        chunks[i].start = (void*)temp_counter;
        chunks[i].size = CHUNK_SIZE;
        chunks[i].occupied = false;
        temp_counter += CHUNK_SIZE;
    }
    nchunks = i;
    return 0;
}

void altmem_dump(void)
{
    for (uint64_t i = 0; i < nchunks; i++) {
        alt_printf("0x%X: chunk{start:0x%X,size:0x%X,occupied:%s}\n", i, (uint64_t)chunks[i].start, chunks[i].size, chunks[i].occupied ? "true" : "false");
    }
    return;
}

void * altmem_alloc(size_t size)
{
    /* Does not handle requests larger than CHUNK_SIZE */ 
    if (size > CHUNK_SIZE) {
        return NULL;
    }

    /* Looks for the first available chunk */ 
    for (uint64_t i = 0; i < nchunks; i++) {
        if (chunks[i].occupied) {
            continue;
        }
        chunks[i].occupied = true;
        return chunks[i].start;
    }

    return NULL;
}

void altmem_free(void * ptr)
{
    /* Looks for chunk with matching start address */ 
    for (uint64_t i = 0; i < nchunks; i++) {
        if (chunks[i].start != ptr) {
            continue;
        }
        chunks[i].occupied = false;
        return;
    }
    
    return;
}

int altmem_findchunk(void * addr)
{
    /* Looks for chunk containing the address */ 
    for (uint64_t i = 0; i < nchunks; i++) {
        if (addr < chunks[i].start || addr >= chunks[i].start + chunks[i].size) {
            continue;
        }
        return i; 
    }

    return -1; /* address does not match an allocatable chunk */

}

int altmem_isoccupied(int i)
{
    if (i < 0 || i >= nchunks ) {
        return -1; // no chunk found
    }

    return chunks[i].occupied ? 1 : 0; // 1 if occupied, 0 if not occupied 
}

bool altmem_stackexpansion(void * ptr)
{
    int i = altmem_findchunk(ptr);
    if (altmem_isoccupied(i) != 0) {
        return false;
    }

    return chunks[i].occupied = true;
}

void * altmem_get_start(int i)
{
    if (i < 0 || i >= nchunks ) {
        return NULL; // no chunk found
    }
    return chunks[i].start;
}

uint64_t altmem_get_size(int i)
{
    if (i < 0 || i >= nchunks ) {
        return 0; // no chunk found
    }
    return chunks[i].size;
}
