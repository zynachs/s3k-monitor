PROGRAM=monitor
INC=-I../common

vpath %.c ../common
vpath %.S ../common

CSRC=main.c s3k.c
ASSRC=start.S common.S
LDS=./monitor.lds
BASE_ADDR=0x80010000

all: ${PROGRAM}.elf ${PROGRAM}.bin

include ../config.mk
