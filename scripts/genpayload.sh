#!/usr/bin/env bash

function cleanup() {
    rm -f $TEMPOBJ $TEMPBIN
}

trap cleanup EXIT

RISCV_PREFIX="riscv64-unknown-elf-"
PAYLOAD_DIR=$(dirname -- "${BASH_SOURCE[0]}")/payloads
PAYLOADS=$(find $PAYLOAD_DIR -type f)
TEMPOBJ=$(mktemp)
TEMPBIN=$(mktemp)

for f in $PAYLOADS; do
	# Compile object file
	"$RISCV_PREFIX"gcc -c $f -o $TEMPOBJ
	
	# Grab the raw code
	"$RISCV_PREFIX"objcopy -O binary $TEMPOBJ $TEMPBIN
	
	# Output the raw code into a series of bytes in comma-seperated 0x notation
    echo -en "FILE: $f\nOUTPUT:\n\t"
	hexdump -v -e '16/1 "0x%02X," "\n\t"' $TEMPBIN 
    echo 
done
