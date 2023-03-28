PROGRAM=app
INC=-I../common

vpath %.c ../common
vpath %.S ../common

CSRC=app.c s3k-utils.c s3k-syscall.c printf.c
ASSRC=start.S common.S
LDS=./app.lds
BASE_ADDR=0x80020000

include ../config.mk
