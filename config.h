// See LICENSE file for copyright and license details.
#pragma once

/* Number of processes. */
#define NPROC 8

/* Number of capabilities per process */
#define NCAP 64

/* Number of time slices in a major frame. */
#define NSLICE 64

/* Number of ticks per quantum. */
/* TICKS_PER_SECOND defined in platform.h */
#define TICKS_PER_SECOND 1000000ull
#define NTICK (TICKS_PER_SECOND / NSLICE / 10ull)

/* Number of scheduler ticks. */
#define NSLACK 100

/* Number of IPC channels */
#define NCHANNEL 8 

#define NDEBUG

/* Uncomment to enable memory protection */
// #define MEMORY_PROTECTION
