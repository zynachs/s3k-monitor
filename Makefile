.POSIX:

RISCV_PREFIX ?= riscv64-unknown-elf-
CC=${RISCV_PREFIX}gcc
OBJCOPY=${RISCV_PREFIX}objcopy
OBJDUMP=${RISCV_PREFIX}objdump

BUILD?=build
S3K_PATH?=../s3k
CONFIG_H?=misc/config.h
COMMON_SRC?=$(wildcard common/*)

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

# Run 
run: all
	./run.sh

# Prints build settings
options:
	@printf "build options:\n"
	@printf "CC       = ${CC}\n"
	@printf "OCFLAGS  = ${OCFLAGS}\n"
	@printf "LDFLAGS  = ${LDFLAGS}\n"
	@printf "ASFLAGS  = ${ASFLAGS}\n"
	@printf "CFLAGS   = ${CFLAGS}\n"
	@printf "S3K_PATH = ${S3K_PATH}\n"
	@printf "CONFIG_H = ${abspath ${CONFIG_H}}\n"

# Clean all
clean_deep: clean clean_s3k

clean:
	rm -rf $(BUILD)
	rm -rf inc/s3k.h
	rm -rf lib/libs3k.a
	$(MAKE) -C tools clean

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
	./tools/qemu.sh $(BUILD)/s3k.elf $(BUILD)/monitor.bin

# Generates payload used in app, see tools/genpayload.sh and tools/gensig.sh
genpayload:
	$(MAKE) -C tools genpayload

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
MONITOR_SRCS=$(wildcard monitor/*) $(COMMON_SRC)
MONITOR_LDS=misc/default.lds
MONITOR_DEPS+=$(patsubst %, $(BUILD)/%.d, $(MONITOR_SRCS))
build/monitor/payload.S.o: $(BUILD)/app.fmt.bin
$(BUILD)/monitor.elf: $(patsubst %, $(BUILD)/%.o, $(MONITOR_SRCS)) lib/libs3k.a 
	@mkdir -p ${@D}
	@printf "CC $@\n"
	@$(CC) ${CLIFLAG} $(LDFLAGS) -T$(MONITOR_LDS) -o $@ $^

# App
APP_SRCS=$(wildcard app/*) $(COMMON_SRC)
APP_LDS=misc/pmp_compatible.lds
APP_DEPS+=$(patsubst %, $(BUILD)/%.d, $(APP_SRCS))
$(BUILD)/app.elf: $(patsubst %, $(BUILD)/%.o, $(APP_SRCS)) lib/libs3k.a
	@mkdir -p ${@D}
	@printf "CC $@\n"
	@$(CC) ${CLIFLAG} $(LDFLAGS) -T$(APP_LDS) -o $@ $^

format_tools:
	$(MAKE) -C tools all

$(BUILD)/app.txt: $(BUILD)/app.elf 
	python3 tools/getsections.py $(BUILD)/app.elf

# Format app
$(BUILD)/app.fmt.bin: format_tools $(BUILD)/app.txt $(BUILD)/app.bin
	tools/build/app_format.elf $(BUILD)/app.bin

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

# Create assembly dump from elf
%.da: %.elf
	@printf "OBJDUMP $< $@\n"
	@${OBJDUMP} -d $< > $@

# Create debug file from elf
%.dbg: %.elf
	@printf "OBJCOPY $< $@\n"
	@${OBJCOPY} --only-keep-debug $< $@

.PHONY: all api clean clean_repo clean_s3k qemu s3k.elf genpayload format_tools app_format

-include $(MONITOR_DEPS) $(APP_DEPS)
