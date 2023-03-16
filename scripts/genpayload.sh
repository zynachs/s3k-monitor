#!/usr/bin/env bash

RISCV_PREFIX="riscv64-unknown-elf-"
PAYLOAD=$1

if [ -z "$PAYLOAD" ]; then
    echo "PAYLOAD missing."
    exit 1
fi

# Compile object file
"$RISCV_PREFIX"gcc -c $1 -o temp.o

# Grab the raw code
"$RISCV_PREFIX"objcopy -O binary temp.o temp.bin

# Output the raw code into a series of bytes in comma-seperated 0x notation
hexdump -v -e '16/1 "0x%02X," "\n"' temp.bin

# Clean up
rm -f temp.o temp.bin