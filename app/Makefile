PROGRAM=app
INC=-I../common

vpath %.c ../common
vpath %.S ../common

CSRC=main.c s3k-utils.c s3k-syscall.c
ASSRC=start.S common.S
LDS=./app.lds
BASE_ADDR=0x80010000

all: ${PROGRAM}.elf ${PROGRAM}.bin

include ../config.mk
