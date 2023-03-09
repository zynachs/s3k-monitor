// See LICENSE file for copyright and license details.
#pragma once

// Start of payload
#define PAYLOAD 0x80010000

/* Number of harts */
#define NHART	4

/* Number of processes. */
#define NPROC 8

/* Number of capabilities per process */
#define NCAP 64

/* Number of time slices in a major frame. */
#define NSLICE 64

/* Number of ticks per quantum. */
/* TICKS_PER_SECOND defined in platform.h */
#define NTICK 1000

/* Number of scheduler ticks. */
#define NSLACK 100

/* Number of IPC channels */
#define NCHANNEL 16

#define NDEBUG 0
