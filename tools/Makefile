.POSIX:

BUILD?=build

.PHONY: all clean payload_dir genpayload

all: payload_dir $(BUILD)/app_format.elf $(BUILD)/app_format_sig.elf

clean:
	rm -rf $(BUILD)
	rm -f codeauth.c
	rm -f aes128.c

payload_dir:
	@mkdir -p $(BUILD)/payloads

genpayload: $(BUILD)/app_format_sig.elf payload_dir
	./genpayload.sh payloads build/payloads

codeauth.c:
	cp ../monitor/codeauth.c .

aes128.c:
	cp ../monitor/aes128.c .

FORMAT_SRCS=app_format.c codeauth.c aes128.c
FORMAT_OBJS=$(patsubst %.c,$(BUILD)/%.o,$(FORMAT_SRCS))
FORMAT_DEPS=$(patsubst %.c,$(BUILD)/%.d,$(FORMAT_SRCS))
# Make tool for formating app binaries with header
$(BUILD)/app_format.elf: $(FORMAT_OBJS)
	@mkdir -p $(BUILD)
	gcc -o $@ $^

SIG_SRCS=app_format_sig.c codeauth.c aes128.c
SIG_OBJS=$(patsubst %.c,$(BUILD)/%.o,$(SIG_SRCS))
SIG_DEPS=$(patsubst %.c,$(BUILD)/%.d,$(SIG_SRCS))
# Make tool for formating code with signature
$(BUILD)/app_format_sig.elf: $(SIG_OBJS)
	@mkdir -p $(BUILD)
	gcc -o $@ $^

$(BUILD)/%.o: %.c
	@mkdir -p $(BUILD)
	gcc -MMD -I../inc -c -o $@ $<

-include $(FORMAT_DEPS) $(SIG_DEPS)
