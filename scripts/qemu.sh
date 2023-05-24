#!/usr/bin/env bash

KERNEL=$1
PAYLOAD=$2
GDBINIT=$(mktemp)
BREAKPOINT_FILE=$(dirname -- "${BASH_SOURCE[0]}")/breakpoints.txt
TMUX_SESSION=s3k-monitor

if ! command -v tmux &> /dev/null; then
    echo "tmux could not be found. This script requires tmux to run."
    exit 1
fi

if [ -z "$KERNEL" ]; then
    echo "Arg 1, KERNEL elf, missing."
    exit 1
fi

if [ -z "$PAYLOAD" ]; then
    echo "Arg 2, PAYLOAD1 elf, missing."
    exit 1
fi

# Finds all files ending with .dbg
# DEBUGFILES=$(for f in $(find build/ -type f -name *.dbg); do echo -e "add-symbol-file $f"; done)                       

# Initial breakpoints. To add a new breakpoint, append a line before the last "EOL". Only one breakpoint per line. 
BREAKPOINTS=$(sed -e 's/^\s*/b /' < $BREAKPOINT_FILE)

# Generates config file 
echo -en "\
set confirm off                           
set pagination off                        
file $KERNEL
add-symbol-file build/monitor.dbg 0x80010000
add-symbol-file build/app.dbg 0x80020000
$BREAKPOINTS
target remote localhost:1234              
layout split                              
fs cmd
" > $GDBINIT

# Start qemu in the background
tmux new-session -d -s $TMUX_SESSION "qemu-system-riscv64 \
        -M virt \
        -smp 1 \
        -m 1G \
        -nographic \
        -bios none \
        -kernel $KERNEL \
        -s \
        -S \
        -device loader,file=$PAYLOAD,addr=0x80010000" &
        #-serial tcp:localhost:4321,server,nowait

sleep 1

# Connect gdb with qemu
tmux split-pane -bfh -p75 "riscv64-unknown-elf-gdb -x $GDBINIT"

# Print help message in tmux
tmux split-pane -bv -l2 -t 0 'echo -ne "Welcome to the debugging live-session of s3k-monitor!\n\nRight panel is GDB prompt, left panel is output from s3k.\nThis is a tmux shell, to exit press Ctrl+d\nTo exit this message press Ctrl+c, or wait 30 seconds." && sleep 30'

tmux attach-session -t $TMUX_SESSION

# Cleanup
echo -e "tmux kill-session -t $TMUX_SESSION"
tmux kill-session -t $TMUX_SESSION
echo -e "rm -f $GDBINIT"
rm -f $GDBINIT
