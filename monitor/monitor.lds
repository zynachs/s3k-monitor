OUTPUT_ARCH("riscv")

ENTRY(_start)

/* Definition of memory region */
MEMORY {
        MONITOR (rwaxi) : ORIGIN  = 0x80010000, LENGTH = 64K
        NOINIT (rwaxi) : ORIGIN = 0x80020000, LENGTH = 64K 
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
        } > MONITOR

        /* Program data 
           OFFSET: +0x4000 (16K) 
        */
        .data : ALIGN(16K) {
                _rodata = .;
                *(.rodata .rodata.*)
                . = _rodata == . ? . + 8K : ALIGN(8K);
                _srodata = .;
                *(.srodata .srodata*)
                . = _srodata == . ? . + 4K : ALIGN(4K);
                _data = .;
                *(.data .data.*)
                . = _data == . ? . + 2K : ALIGN(2K);
                _sdata = .;
                *(.sdata .sdata.*)
                . = _sdata == . ? . + 2K : ALIGN(2K);
                _edata = .;
        } > MONITOR

        /* Zero initialized region
           OFFSET: +0x8000 (32K)
        */
        .bss : ALIGN(4K) {
                _bss = .;
                *(.sbss .sbss.*)
                *(.bss .bss.*)
                _ebss = ALIGN(8);
        } > MONITOR

        /* Program stack
           OFFSET: +0xF800 (62K)
        */
        .stack : ALIGN(4K) {
                . = LENGTH(MONITOR) + ORIGIN(MONITOR) - 2K;
                *(.stack .stack.*)
        } > MONITOR

        /* Reserved for app 
           OFFSET: +0x10000 (64K)
        */
        .noinit : {
                _noinit = .;
                *(.noinit .noinit.*)
                _enoinit = .;
        } > NOINIT

        /* Hardcoded to start of .bss since it in the middle between .data and .bss */
        __global_pointer$ = _bss;
        
        /* End of App (0x80030000) 
           OFFSET: +0x20000 (64K)
        */
	_end = .;
}