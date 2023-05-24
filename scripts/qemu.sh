#!/usr/bin/env bash

# Cleanup
function cleanup() {
    echo -e "Exiting... cleaning up:"
    echo -e "  $ tmux kill-session -t $TMUX_SESSION"
    tmux kill-session -t $TMUX_SESSION
    echo -e "  $ rm -f $GDBINIT"
    rm -f $GDBINIT   
}

trap cleanup EXIT

SOURCE_DIR=$(dirname -- "${BASH_SOURCE[0]}")
KERNEL=$1
MONITOR=$2
MONITOR_DBG=${MONITOR%.*}.dbg
APP_DBG=$SOURCE_DIR/../build/app.dbg
GDBINIT=$(mktemp)
BREAKPOINT_FILE=$SOURCE_DIR/breakpoints.txt
TMUX_SESSION=s3k-monitor

if ! command -v tmux &> /dev/null; then
    echo "ERROR: tmux could not be found. This script requires tmux to run."
    exit 1
fi

if [ -z "$KERNEL" ]; then
    echo "ERROR: Arg 1, KERNEL elf, missing."
    exit 1
fi

if [ -z "$MONITOR" ]; then
    echo "ERROR: Arg 2, MONITOR elf, missing."
    exit 1
fi

# Initial breakpoints. To add a new breakpoint, append a line before the last "EOL". Only one breakpoint per line. 
BREAKPOINTS=$(sed -e 's/^\s*/b /' < $BREAKPOINT_FILE)

# Generates config file 
echo -en "\
set confirm off                           
set pagination off                        
file $KERNEL
add-symbol-file $MONITOR_DBG 0x80010000
add-symbol-file $APP_DBG 0x80020000
$BREAKPOINTS
target remote localhost:1234              
layout split                              
fs cmd
" > $GDBINIT

# Start qemu in the background
tmux new-session -d -s $TMUX_SESSION -n debug \
    "qemu-system-riscv64 \
        -M virt \
        -smp 1 \
        -m 128M \
        -nographic \
        -bios none \
        -kernel $KERNEL \
        -s \
        -S \
        -device loader,file=$MONITOR,addr=0x80010000"
        #-serial tcp:localhost:4321,server,nowait

sleep 1

# Connect gdb with qemu
tmux split-pane -bfh -p75 "riscv64-unknown-elf-gdb -x $GDBINIT"

# Print help message in tmux
tmux new-window -n help 'echo -ne "\
Welcome to the live debug session of s3k-monitor!
=================================================

Quick-start guide:
    * Right panel is GDB prompt, left panel is output from s3k.
    * This is a tmux shell, to exit press Ctrl + b + release keys + d.
    * To leave this message press Ctrl + b + release keys + n, repeat combination to return to this message.

Tips:
    * Mouse controls are enabled so you can click and scroll to navigate.
    * Copy text by highlighting the desired text by clickling and draging (does not work in GDB tui mode). 
    * If the text in GDB becomes garbled, press Ctrl + b + release keys + g.
    * Restart kernel and debugging session by pressing Ctrl + b + release keys + r.

" | less -'

# Enable mouse controls in tmux
tmux set -g mouse

# Create a macros to restart session and refresh GDB screen
tmux bind-key r respawn-pane -k -t 1 \\\; run-shell "sleep 1" \\\; respawn-pane -k -t 0
tmux bind-key g send-keys C-x "a" \\\; send-keys C-x "a"

# Attach to tmux session
tmux attach-session -t $TMUX_SESSION
