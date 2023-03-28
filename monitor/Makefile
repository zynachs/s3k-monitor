PROGRAM=monitor
INC=-I../common

vpath %.c ../common
vpath %.S ../common

CSRC=monitor.c s3k-utils.c s3k-syscall.c printf.c
ASSRC=start.S common.S payload.S
LDS=./monitor.lds
BASE_ADDR=0x80010000

OBJCOPY_DBG=-R .noinit

include ../config.mk
