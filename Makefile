.POSIX:

export RISCV_PREFIX ?= riscv64-unknown-elf-
export CC=${RISCV_PREFIX}gcc
export OBJCOPY=${RISCV_PREFIX}objcopy
export OBJDUMP=${RISCV_PREFIX}objdump

# MONITOR=monitor/monitor.bin
# APP=app/app.bin
# KERNEL=../s3k/build/s3k.elf
BUILD?=build
S3K_PATH?=../s3k
CONFIG_H?=config.h
PAYLOAD=./scripts/payloads/malicious.c # Malicious payload used in app

# Compilation flags
CFLAGS+=\
	-march=rv64imac \
	-mabi=lp64 \
	-mcmodel=medany \
	-std=c11 \
	-Wall \
	-Werror \
	-g \
	-Os \
	-ffreestanding \
	-Iinc

# Assembler flags
ASFLAGS=\
	-march=rv64imac \
	-mabi=lp64 \
	-mcmodel=medany\
	-g\
	-ffreestanding

# Linker flags
LDFLAGS=\
	-march=rv64imac \
	-mabi=lp64 \
	-mcmodel=medany\
	-Wl,--gc-sections,--no-dynamic-linker\
	-Wstack-usage=2048 \
	-fstack-usage\
	-nostartfiles
#-T${LDS}

# Build all
all: options $(BUILD)/monitor.bin $(BUILD)/s3k.elf

# Prints build settings
options:
	@printf "build options:\n"
	@printf "CC       = ${CC}\n"
	@printf "LDFLAGS  = ${LDFLAGS}\n"
	@printf "ASFLAGS  = ${ASFLAGS}\n"
	@printf "CFLAGS   = ${CFLAGS}\n"
	@printf "S3K_PATH = ${S3K_PATH}\n"
	@printf "CONFIG_H = ${abspath ${CONFIG_H}}\n"

# Clean all
clean: clean_repo clean_s3k

clean_repo:
	rm -rf $(BUILD)

clean_s3k:
	${MAKE} -C ../s3k clean

# API
api: inc/s3k.h lib/libs3k.a

inc/s3k.h: $(S3K_PATH)/api/s3k.h
	cp $(S3K_PATH)/api/s3k.h inc/s3k.h

lib/libs3k.a: $(wildcard $(S3K_PATH)/api/*.c) inc/s3k.h
	$(MAKE) -C $(S3K_PATH)/api libs3k.a
	cp $(S3K_PATH)/api/libs3k.a lib/libs3k.a

# Run with qemu
qemu: $(BUILD)/s3k.elf $(BUILD)/monitor.bin
	./scripts/qemu.sh $^

# Generates payload used in app, see ./scripts/genpayload.sh
genpayload: ${PAYLOAD} 
	./scripts/genpayload.sh $(PAYLOAD)

# Build .o files from .c files
$(BUILD)/%.c.o: %.c
	@mkdir -p ${@D}
	@printf "CC $@\n"
	@$(CC) $(CFLAGS) -MMD -c -o $@ $<

# Build .o files from .S files  
$(BUILD)/%.S.o: %.S
	@mkdir -p ${@D}
	@printf "CC $@\n"
	@$(CC) $(ASFLAGS) -MMD -c -o $@ $<

# Monitor 
SRCS=monitor/monitor.c monitor/capman.c monitor/payload.S common/start.S
LDS=default.ld
DEPS+=$(patsubst %, $(BUILD)/%.d, $(SRCS))
build/monitor/payload.S.o: build/app.bin
$(BUILD)/monitor.elf: $(patsubst %, $(BUILD)/%.o, $(SRCS)) lib/libs3k.a
	@mkdir -p ${@D}
	@printf "CC $@\n"
	@$(CC) $(LDFLAGS) -T$(LDS) -o $@ $^

# App
SRCS=app/app.c common/start.S
LDS=default.ld
DEPS+=$(patsubst %, $(BUILD)/%.d, $(SRCS))
$(BUILD)/app.elf: $(patsubst %, $(BUILD)/%.o, $(SRCS)) lib/libs3k.a
	@mkdir -p ${@D}
	@printf "CC $@\n"
	@$(CC) $(LDFLAGS) -T$(LDS) -o $@ $^

# Make kernel
$(BUILD)/s3k.elf: ${CONFIG_H}
	@mkdir -p ${@D}
	@${MAKE} -C ${S3K_PATH} options kernel  \
		CONFIG_H=${abspath ${CONFIG_H}} \
		BUILD_DIR=$(abspath $(BUILD))   \
		OBJ_DIR=$(abspath $(BUILD))/s3k

# Create bin file from elf
%.bin: %.elf
	@printf "OBJCOPY $< $@\n"
	@${OBJCOPY} -O binary $< $@

# Create assebly dump
%.da: %.elf
	@printf "OBJDUMP $< $@\n"
	@${OBJDUMP} -d $< > $@

####
debug: app monitor
	${MAKE} -C ${word 1,$^} ${word 1,$^}.dbg
	${MAKE} -C ${word 2,$^} ${word 2,$^}.dbg
###

.PHONY: all api clean clean_repo clean_s3k qemu s3k.elf genpayload

-include $(DEPS)
