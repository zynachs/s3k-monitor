OUTPUT_ARCH("riscv")

ENTRY(_start)

/* Definition of memory region */
MEMORY {
        MONITOR (rwaxi) : ORIGIN  = 0x80010000, LENGTH = 64K
}

SECTIONS {
        /* All regions are statically aligned to 4K to be compatible with PMP */

        /* Program instructions */
        .text : ALIGN(4K) {
                _text = .;
                *(.init)
                *(.text .text.*)
                _etext = .;
        } > MONITOR

        /* Program data */
        .data : ALIGN(4K) {
                _data = .;
                *(.data .data.*)
                . = _data == . ? . + 1K : ALIGN(1K);
                _sdata = .;
                *(.sdata .sdata.*)
                . = _sdata == . ? . + 1K : ALIGN(1K);
                _rodata = .;
                *(.rodata .rodata.*)
                . = _rodata == . ? . + 1K : ALIGN(1K);
                _srodata = .;
                *(.srodata .srodata*)
                _edata = .;
        } > MONITOR

        /* Zero initialized region*/
        .bss : ALIGN(4K) {
                _bss = .;
                *(.sbss .sbss.*)
                *(.bss .bss.*)
                _ebss = ALIGN(8);
        } > MONITOR

        /* Program stack*/
        .stack : {
                . = (ORIGIN(MONITOR) + LENGTH(MONITOR)) - 2K;
                *(.stack .stack.*)
        } > MONITOR

	_end = .;

        /* Hardcoded to start of .bss since it in the middle between .data and .bss */
        __global_pointer$ = _bss;
}