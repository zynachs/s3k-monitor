.POSIX:

export RISCV_PREFIX ?= riscv64-unknown-elf-
export CC=${RISCV_PREFIX}gcc
export OBJCOPY=${RISCV_PREFIX}objcopy
export OBJDUMP=${RISCV_PREFIX}objdump

BUILD?=build
S3K_PATH?=../s3k
CONFIG_H?=misc/config.h

# flags passed in cmd
CLIFLAG=
ifeq (${CLIFLAG},debug)
	CFLAGS+=-DDEBUG
else ifeq (${CLIFLAG},test1)
	CFLAGS+=-D__TEST1
else ifeq (${CLIFLAG},test2)
	CFLAGS+=-D__TEST2
else ifeq (${CLIFLAG},test3)
	CFLAGS+=-D__TEST3
endif

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
	-nostartfiles \
	-Iinc

# Objcopy flags. Used to deflate the size of the binary files since they will contain a lot of unused space.
OCFLAGS=\
	-R .bss \
	-R .stack 

# Build all
all: options api $(BUILD)/monitor.bin $(BUILD)/s3k.elf debug

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
	rm inc/s3k.h
	rm lib/libs3k.a

clean_s3k:
	${MAKE} -C ../s3k clean

# API
api: inc/s3k.h lib/libs3k.a

inc/s3k.h: $(S3K_PATH)/api/s3k.h
	@mkdir -p $(@D)
	cp $(S3K_PATH)/api/s3k.h inc/s3k.h

lib/libs3k.a: $(wildcard $(S3K_PATH)/api/*.c) inc/s3k.h
	@mkdir -p $(@D)
	$(MAKE) -C $(S3K_PATH)/api libs3k.a
	cp $(S3K_PATH)/api/libs3k.a lib/libs3k.a

# Run with qemu
qemu: all
	./scripts/qemu.sh $(BUILD)/s3k.elf $(BUILD)/monitor.bin

# Generates payload used in app, see ./scripts/genpayload.sh
genpayload:
	./scripts/genpayload.sh

# Generate debug files
debug: $(BUILD)/monitor.dbg $(BUILD)/app.dbg

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
LDS=monitor/monitor.lds
DEPS+=$(patsubst %, $(BUILD)/%.d, $(SRCS))
build/monitor/payload.S.o: build/app.bin
$(BUILD)/monitor.elf: $(patsubst %, $(BUILD)/%.o, $(SRCS)) lib/libs3k.a
	@mkdir -p ${@D}
	@printf "CC $@\n"
	@$(CC) ${CLIFLAG} $(LDFLAGS) -T$(LDS) -o $@ $^

# App
SRCS=app/app.c common/start.S
LDS=app/app.lds
DEPS+=$(patsubst %, $(BUILD)/%.d, $(SRCS))
$(BUILD)/app.elf: $(patsubst %, $(BUILD)/%.o, $(SRCS)) lib/libs3k.a
	@mkdir -p ${@D}
	@printf "CC $@\n"
	@$(CC) ${CLIFLAG} $(LDFLAGS) -T$(LDS) -o $@ $^

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
	@${OBJCOPY} ${OCFLAGS} -O binary $< $@

# Create assebly dump from elf
%.da: %.elf
	@printf "OBJDUMP $< $@\n"
	@${OBJDUMP} -d $< > $@

# Create debug file from elf
%.dbg: %.elf
	@printf "OBJCOPY $< $@\n"
	@${OBJCOPY} --only-keep-debug $< $@

.PHONY: all api clean clean_repo clean_s3k qemu s3k.elf genpayload

-include $(DEPS)
