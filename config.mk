OBJ=${addprefix obj/, ${CSRC:.c=.o} ${ASSRC:.S=.o}}
DEP=${OBJ:.o:.d}
CLIFLAG=

# Compilation flags
CFLAGS+=-march=rv64imac -mabi=lp64 -mcmodel=medany\
	-std=c11\
	-Wall -Werror\
	-g -Os\
	-ffreestanding

ifeq (${CLIFLAG},debug)
	CFLAGS+=-DDEBUG
else ifeq (${CLIFLAG},test1)
	CFLAGS+=-D__TEST1
else ifeq (${CLIFLAG},test2)
	CFLAGS+=-D__TEST2
else ifeq (${CLIFLAG},test3)
	CFLAGS+=-D__TEST3
endif

ASFLAGS=-march=rv64imac -mabi=lp64 -mcmodel=medany\
	-g\
	-ffreestanding

LDFLAGS=-march=rv64imac -mabi=lp64 -mcmodel=medany\
	-Wl,--gc-sections,--no-dynamic-linker\
	-Wstack-usage=2048 -fstack-usage\
	-nostartfiles\
	-T${LDS}

# Remove zeroed regions from binary to save space
OCFLAGS=-R .bss -R .stack

obj:
	mkdir -p $@

${OBJ}: ${LDS} | obj

obj/%.o: %.c
	${CC} ${CFLAGS} ${INC} ${DEFS} -MMD -c -o $@ $<

obj/%.o: %.S
	${CC} ${ASFLAGS} ${INC} ${DEFS} -MMD -c -o $@ $<

${PROGRAM}.elf: ${OBJ} ${LDS}
	${CC} ${LDFLAGS} -o $@ ${OBJ}

${PROGRAM}.bin: ${PROGRAM}.elf
	${OBJCOPY} ${OCFLAGS} -O binary $< $@

-include ${DEP}
