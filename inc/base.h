#pragma once

#define MONITOR_BASE 0x80010000ull
#define APP_BASE 0x80020000ull
#define APP_SIZE 0x10000ull
#define CHUNK_SIZE 0x1000ull
#define CHUNK_MASK 0x601Full /* 0b1100000000111111, only seven chunks enabled to fit UART PMP. */ 
