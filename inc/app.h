#pragma once

#include <stdint.h>

extern const void * _start; /* base address of app, defined in start.S */
extern const void * _stack_top; /* address of stack top, defined in start.S */
extern const void * _stack; /* address of bottom of stack, defined in start.S */
extern const void * _heap;
extern const void * _rodata;
extern const void * _text;

extern uint8_t pmpcaps[2][8];
extern uint8_t soccaps[8];
extern uint64_t buf[4];
extern uint64_t tag;

void setup();
void loop();
