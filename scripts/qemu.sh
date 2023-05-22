#!/usr/bin/env bash

function cleanup() {
    rm -f $GDBINIT
    kill $(jobs -p)
}

trap cleanup EXIT

KERNEL=$1
PAYLOAD=$2
GDBINIT=$(mktemp)
BREAKPOINT_FILE=$(dirname -- "${BASH_SOURCE[0]}")/breakpoints.txt

if [ -z "$KERNEL" ]; then
    echo "Arg 1, KERNEL elf, missing."
    exit 1
fi

if [ -z "$PAYLOAD" ]; then
    echo "Arg 2, PAYLOAD1 elf, missing."
    exit 1
fi

# Start qemu in background
qemu-system-riscv64 \
        -M virt \
        -smp 1 \
        -m 1G \
        -nographic \
        -bios none \
        -kernel $KERNEL \
        -s \
        -S \
        -device loader,file=$PAYLOAD,addr=0x80010000 \
        -serial tcp:localhost:4321,server,nowait &

# Finds all files ending with .dbg
DEBUGFILES=$(for f in $(find -type f -name *.dbg); do echo -e "add-symbol-file $f"; done)                       

# Initial breakpoints. To add a new breakpoint, append a line before the last "EOL". Only one breakpoint per line. 
BREAKPOINTS=$(sed -e 's/^\s*/b /' < $BREAKPOINT_FILE)

# Generates config file 
echo -en "\
set confirm off                           
set pagination off                        
file $KERNEL
$DEBUGFILES
$BREAKPOINTS
target remote localhost:1234              
layout split                              
fs cmd
" > $GDBINIT

riscv64-unknown-elf-gdb -x $GDBINIT
