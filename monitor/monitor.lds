OUTPUT_ARCH("riscv")

ENTRY(_start)

/* Definition of memory region */
MEMORY {
        MONITOR (rwaxi) : ORIGIN  = 0x80010000, LENGTH = 64K
}

SECTIONS {
        /* All regions are statically aligned to be compatible with PMP */

        /* Program instructions 
           OFFSET: +0x00
           MAXSIZE: 8K
        */
        .text : { 
                _text = .;
                *(.init)
                *(.text .text.*)
                _etext = .;
        } > MONITOR

        ASSERT ((SIZEOF(.text) <= 8K), "ERROR: .text region too large, will break PMP compatability.")

        /* Program data 
           OFFSET: +0x2000 (8K) 
           MAXSIZE: 8K
        */
        .data : ALIGN(8K) {
                _data = .;
                *(.data .data.*)
                _sdata = .;
                *(.sdata .sdata.*)
                _edata = .;
                . = _data == . ? . + 8K : .; /* if .data is empty move position forward anyways */
        } > MONITOR
        
        ASSERT ((SIZEOF(.data) <= 8K), "ERROR: .data region too large, will break PMP compatability.")

        /* Program read-only data
           OFFSET: +0x4000 (16K) 
           MAXSIZE: 16K
        */
        .rodata : ALIGN(16K) {
                _rodata = .;
                *(.rodata .rodata.*)
                _srodata = .;
                *(.srodata .srodata*)
                _erodata = .;
                . = _rodata == . ? . + 16K : .; /* if .rodata is empty move position forward anyways */
        }

        ASSERT ((SIZEOF(.rodata) <= 16K), "ERROR: .rodata region too large, will break PMP compatability.")

        /* Zero initialized region
           OFFSET: +0x8000 (32K)
           MAXSIZE: 4K
        */
        .bss : ALIGN(16K) {
                _bss = .;
                *(.sbss .sbss.*)
                *(.bss .bss.*)
                _ebss = ALIGN(8);
        } > MONITOR
        
        ASSERT ((SIZEOF(.bss) <= 4K), "ERROR: .bss region too large, will break PMP compatability.")
        
        /* Program stack
           OFFSET: +0xF800 (62K)
           MAXSIZE: 2K
        */
        .stack : ALIGN(4K) {
                . = LENGTH(MONITOR) + ORIGIN(MONITOR) - 2K;
                *(.stack .stack.*)
        } > MONITOR
        
        /* End of monitor (0x80020000) 
           OFFSET: +0x10000 (64K)
        */
	_end = .;

        /* Hardcoded to start of .bss since it in the middle between .data and .bss */
        __global_pointer$ = _bss;
}
