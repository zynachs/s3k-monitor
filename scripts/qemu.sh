#!/usr/bin/env bash
KERNEL=$1
PAYLOAD=$2
BASEDIR=$(dirname "$0")

if [ -z "$KERNEL" ]; then
    echo "Arg 1, KERNEL elf, missing."
    exit 1
fi

if [ -z "$PAYLOAD" ]; then
    echo "Arg 2, PAYLOAD1 elf, missing."
    exit 1
fi

function cleanup() {
        kill $(jobs -p)
}

trap cleanup EXIT

qemu-system-riscv64 -M virt -smp 1 -m 1G -nographic -bios none -kernel $KERNEL -s -S \
        -device loader,file=$PAYLOAD,addr=0x80010000 \
        -serial tcp:localhost:4321,server,nowait &

echo -en "\
set confirm off                           
set pagination off                        
symbol-file $KERNEL
$(for f in $(ls $BASEDIR/../debug); do echo -e "add-symbol-file debug/$f"; done)                       
b *0x80010000                             
b handle_exception                        
target remote localhost:1234              
layout split                              
fs cmd
" > $BASEDIR/.gdbinit

riscv64-unknown-elf-gdb -x $BASEDIR/.gdbinit
