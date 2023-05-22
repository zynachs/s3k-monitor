#!/usr/bin/env bash

function cleanup() {
    rm -f $TEMPOBJ $TEMPBIN
}

trap cleanup EXIT

RISCV_PREFIX="riscv64-unknown-elf-"
PAYLOAD=$1
TEMPOBJ=$(mktemp)
TEMPBIN=$(mktemp)

if [ -z "$PAYLOAD" ]; then
    echo "PAYLOAD missing."
    exit 1
fi

# Compile object file
"$RISCV_PREFIX"gcc -c $1 -o $TEMPOBJ

# Grab the raw code
"$RISCV_PREFIX"objcopy -O binary $TEMPOBJ $TEMPBIN

# Output the raw code into a series of bytes in comma-seperated 0x notation
hexdump -v -e '16/1 "0x%02X," "\n"' $TEMPBIN
