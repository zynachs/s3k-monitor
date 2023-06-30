#pragma once

#define MONITOR_BASE 0x80010000ull
#define APP_BASE 0x80020000ull
#define APP_SIZE 0x10000ull
#define CHUNK_SIZE 0x1000ull
#define CHUNK_MASK 0x601Full /* 0b1100000000111111, only seven chunks enabled to fit UART PMP. */ 

/*
#define BOOT_BEGIN 0x80010000ull
#define MONITOR_BASE 0x80020000ull
#define CRYPTO_BASE 0x80024000ull
#define UARTPPP_BASE 0x80028000ull
#define APP0_BASE 0x8002c000ull
#define APP1_BASE 0x80030000ull
#define SHARED_BASE 0x80040000ull
*/
