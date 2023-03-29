.POSIX:

RISCV_PREFIX ?=riscv64-unknown-elf-
export CC=${RISCV_PREFIX}gcc
export OBJCOPY=${RISCV_PREFIX}objcopy
export OBJDUMP=${RISCV_PREFIX}objdump

MONITOR=monitor/monitor.bin
APP=app/app.bin
KERNEL=../s3k/s3k.elf
CONFIG_H=${abspath config.h}
PAYLOAD=./scripts/payloads/malicious.c # Malicious payload used in app

all: ${MONITOR} ${KERNEL}

clean: clean_repo clean_s3k

clean_repo:
	git clean -fdX

clean_s3k:
	${MAKE} -C ${dir ${KERNEL}} clean

common/s3k.h: ../s3k/api/s3k.h
	cp $< $@

common/s3k-utils.c: ../s3k/api/s3k-utils.c
	cp $< $@

common/s3k-syscall.c: ../s3k/api/s3k-syscall.c
	cp $< $@

api: common/s3k.h common/s3k-utils.c common/s3k-syscall.c

debug/monitor.dbg: monitor/monitor.elf.dbg
	mv $< $@

debug/app.dbg: app/app.elf.dbg
	mv $< $@

debug/:
	mkdir $@

debug: debug/ debug/monitor.dbg debug/app.dbg 

${basename ${MONITOR}}.elf.dbg: ${MONITOR}
	${MAKE} -C ${@D} ${@F}

${basename ${APP}}.elf.dbg: ${APP}
	${MAKE} -C ${@D} ${@F}

${MONITOR}: api ${APP}
	${MAKE} -C ${@D} ${@F}

${APP}:
	${MAKE} -C ${@D} ${@F}

${KERNEL}:
	${MAKE} -C ${@D} ${@F} CONFIG_H=${CONFIG_H}

%.bin: %.elf
	${OBJCOPY} -O binary $< $@

%.da: %.elf
	${OBJDUMP} -d $< > $@

qemu: $(KERNEL) $(MONITOR)
	./scripts/qemu.sh $(KERNEL) $(MONITOR)

# Generates payload used in app, see ./scripts/genpayload.sh
genpayload:
	./scripts/genpayload.sh $(PAYLOAD)

.PHONY: all api clean qemu genpayload ${MONITOR} ${KERNEL} debug 
