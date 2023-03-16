OUTPUT_ARCH("riscv")

ENTRY(_start)

/* Definition of memory region */
MEMORY {
        APP (rwaxi) : ORIGIN  = 0x80020000, LENGTH = 64K
}


SECTIONS {
        .text : ALIGN(4K) {
                _text = .;
                *(.init)
                *(.text .text.*)
                _etext = .;
        } > APP

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
        } > APP

        .bss : ALIGN(4K) {
                _bss = .;
                *(.sbss .sbss.*)
                *(.bss .bss.*)
                _ebss = ALIGN(8);
        } > APP

        .stack : {
                . = (ORIGIN(APP) + LENGTH(APP)) - 2K;
                *(.stack .stack.*)
        } > APP

	_end = .;

        __global_pointer$ = _bss;
}