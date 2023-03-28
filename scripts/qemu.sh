#!/usr/bin/env bash
KERNEL=$1
PAYLOAD1=$2

if [ -z "$KERNEL" ]; then
    echo "Arg 1, KERNEL elf, missing."
    exit 1
fi

if [ -z "$PAYLOAD1" ]; then
    echo "Arg 2, PAYLOAD1 elf, missing."
    exit 1
fi

MONITOR=$(mktemp /tmp/XXXXX1.bin)

riscv64-unknown-elf-objcopy -O binary $PAYLOAD1 $MONITOR

function cleanup() {
        rm -f $MONITOR
        rm -f $TMPFIFO
        kill $(jobs -p)
}

trap cleanup EXIT

qemu-system-riscv64 -M virt -smp 1 -m 1G -nographic -bios none -kernel $KERNEL -s -S \
        -device loader,file=$MONITOR,addr=0x80010000 \
        -serial tcp:localhost:4321,server,nowait &

riscv64-unknown-elf-gdb                                 \
        -ex "set confirm off"                           \
        -ex "set pagination off"                        \
        -ex "symbol-file $KERNEL"                       \
        -ex "add-symbol-file $PAYLOAD1"                  \
        -ex "b *0x80010000"                             \
	-ex "b handle_exception"                        \
        -ex "target remote localhost:1234"              \
        -ex "layout split"                              \
        -ex "fs cmd"                                      # tui
