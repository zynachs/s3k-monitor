#!/usr/bin/env bash

function cleanup() {
    rm -f $TEMPOBJ $TEMPBIN
}

trap cleanup EXIT

RISCV_PREFIX="riscv64-unknown-elf-"

PAYLOAD_DIR=$1
BUILD=$2

if [ -z "$PAYLOAD_DIR" ]; then
	echo "PAYLOAD_DIR not provided as an argument."
	exit 1
fi

if [ -z "$BUILD" ]; then
	echo "BUILD not provided as an argument."
	exit 1
fi

PAYLOADS=$(find $PAYLOAD_DIR -type f)
TEMPOBJ=$(mktemp)
TEMPBIN=$(mktemp)

for f in $PAYLOADS; do
	# Final destination for output
	DESTBIN=$BUILD/$(basename $f .c).bin
	SIGBIN=$BUILD/$(basename $f .c).sig.bin

	# Compile object file
	"$RISCV_PREFIX"gcc -c $f -o $TEMPOBJ
	
	# Grab the raw code
	"$RISCV_PREFIX"objcopy -O binary $TEMPOBJ $TEMPBIN

	# Copy binary output to build directory
	cp $TEMPBIN $DESTBIN

	# Create signature for binary output
	build/app_format_sig.elf $DESTBIN
	
	# Output the raw code into a series of bytes in comma-seperated 0x notation
    echo -en "SOURCE: $f\nSIG: $SIGBIN\n\t"
	hexdump -v -n16 -e '16/1 "0x%02X," "\n\t"' $SIGBIN
	echo -en "[...]\n"
	echo -en "BIN: $DESTBIN\n\t"
	hexdump -v -e '16/1 "0x%02X," "\n\t"' $DESTBIN 
    echo
done
