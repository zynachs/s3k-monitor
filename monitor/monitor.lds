OUTPUT_ARCH("riscv")

ENTRY(_start)

MEMORY {
        MONITOR (rwaxi) : ORIGIN  = 0x80010000, LENGTH = 64K
}

SECTIONS {
        .text : ALIGN(4K) {
                _text = .;
                *(.init)
                *(.text .text.*)
                _etext = .;
        } > MONITOR

        .data : ALIGN(4K) {
                _data = .;
                *(.data .data.*)
                . = _data == . ? . + 1K : ALIGN(1K);
                _rodata = .;
                *(.rodata .rodata.*)
                . = _rodata == . ? . + 1K : ALIGN(1K);
                _sdata = .;
                *(.sdata .sdata.*)
                . = _sdata == . ? . + 1K : ALIGN(1K);
                _srodata = .;
                *(.srodata .srodata*)
                _edata = .;
        } > MONITOR

        .bss : ALIGN(4K) {
                _bss = .;
                *(.sbss .sbss.*)
                *(.bss .bss.*)
                _ebss = ALIGN(8);
        } > MONITOR

        .stack : {
                . = (ORIGIN(MONITOR) + LENGTH(MONITOR)) - 8K;
                *(.stack .stack.*)
        } > MONITOR

	_end = .;

        __global_pointer$ = _bss;
}