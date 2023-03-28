OUTPUT_ARCH("riscv")

ENTRY(_start)

/* Definition of memory region */
MEMORY {
        APP (rwaxi) : ORIGIN  = 0x80020000, LENGTH = 64K
}

SECTIONS {
        /* All regions are statically aligned to 4K to be compatible with PMP */

        /* Program instructions 
           OFFSET: +0x00
        */
        .text : ALIGN(4K) { 
                _text = .;
                *(.init)
                *(.text .text.*)
                _etext = .;
        } > APP

        /* Program data 
           OFFSET: +0x1000 (4K) 
        */
        .data : ALIGN(4K) {
                _rodata = .;
                *(.rodata .rodata.*)
                . = _rodata == . ? . + 1K : ALIGN(1K);
                _srodata = .;
                *(.srodata .srodata*)
                . = _srodata == . ? . + 1K : ALIGN(1K);
                _data = .;
                *(.data .data.*)
                . = _data == . ? . + 1K : ALIGN(1K);
                _sdata = .;
                *(.sdata .sdata.*)
                . = _sdata == . ? . + 1K : ALIGN(1K);
                _edata = .;
        } > APP

        /* Zero initialized region
           OFFSET: +0x2000 (8K)
        */
        .bss : ALIGN(4K) {
                _bss = .;
                *(.sbss .sbss.*)
                *(.bss .bss.*)
                _ebss = ALIGN(8);
        } > APP

        /* Program stack
           OFFSET: +0xF800 (62K)
        */
        .stack : ALIGN(4K) {
                . = ORIGIN(APP) + LENGTH(APP) - 2K;
                *(.stack .stack.*)
        } > APP

        /* Hardcoded to start of .bss since it in the middle between .data and .bss */
        __global_pointer$ = _bss;
        
        /* End of App (0x80030000) 
           OFFSET: +0x10000 (64K)
        */
	_end = .;
}
