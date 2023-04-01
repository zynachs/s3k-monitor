.POSIX:

RISCV_PREFIX ?= riscv64-unknown-elf-
export CC=${RISCV_PREFIX}gcc
export OBJCOPY=${RISCV_PREFIX}objcopy
export OBJDUMP=${RISCV_PREFIX}objdump

MONITOR=monitor/monitor.bin
APP=app/app.bin
KERNEL=../s3k/build/s3k.elf
CONFIG_H=${abspath config.h}
PAYLOAD=./scripts/payloads/malicious.c # Malicious payload used in app

all: kernel app monitor

clean: clean_repo clean_s3k

clean_repo:
	git clean -fdX

clean_s3k:
	${MAKE} -C ../s3k clean

common/s3k.h: ../s3k/api/s3k.h
	cp $< $@

common/s3k-utils.c: ../s3k/api/s3k-utils.c
	cp $< $@

common/s3k-syscall.c: ../s3k/api/s3k-syscall.c
	cp $< $@

api: common/s3k.h common/s3k-utils.c common/s3k-syscall.c

debug: app monitor
	${MAKE} -C ${word 1,$^} ${word 1,$^}.dbg
	${MAKE} -C ${word 2,$^} ${word 2,$^}.dbg

app: 
	${MAKE} -C $@ $@.bin

monitor: app
	${MAKE} -C $@ $@.bin

kernel:
	${MAKE} -C ../s3k $@ CONFIG_H=${CONFIG_H}

qemu: all debug
	./scripts/qemu.sh $(KERNEL) ${MONITOR}

# Generates payload used in app, see ./scripts/genpayload.sh
genpayload: ${PAYLOAD} 
	./scripts/genpayload.sh $(PAYLOAD)

.PHONY: all api clean clean_repo clean_s3k qemu genpayload debug monitor app kernel
