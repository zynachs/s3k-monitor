OUTPUT_ARCH("riscv")

ENTRY(_start)

/* Definition of memory region */
MEMORY {
        APP (rwaxi) : ORIGIN  = 0x80020000, LENGTH = 64K
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
                . = ALIGN(8K);
                _etext = .;
        } > APP

        ASSERT ((SIZEOF(.text) <= 8K), "ERROR: .text region too large, will break PMP compatability.")

        /* Program data 
           OFFSET: +0x2000 (8K) 
           MAXSIZE: 4K
        */
        .data : ALIGN(8K) {
                _data = .;
                *(.data .data.*)
                _sdata = .;
                *(.sdata .sdata.*)
                . = _data == . ? . + 4K : .; /* if .data is empty move position forward anyways */
                . = ALIGN(4K);
                _edata = .;
        } > APP

        ASSERT ((SIZEOF(.data) <= 4K), "ERROR: .data region too large, will break PMP compatability.")

        /* Program read-only data
           OFFSET: +0x3000 (12K) 
           MAXSIZE: 4K
        */
        .rodata : ALIGN(4K) {
                _rodata = .;
                *(.rodata .rodata.*)
                _srodata = .;
                *(.srodata .srodata*)
                . = _rodata == . ? . + 4K : .; /* if .rodata is empty move position forward anyways */
                . = ALIGN(4K);
                _erodata = .;
        } > APP
        
        ASSERT ((SIZEOF(.rodata) <= 4K), "ERROR: .rodata region too large, will break PMP compatability.")
        
        /* Zero initialized region
           OFFSET: +0x4000 (16K)
           MAXSIZE: 4K
        */
        .bss : ALIGN(4K) {
                _bss = .;
                *(.sbss .sbss.*)
                *(.bss .bss.*)
                . = _bss == . ? . + 4K : .; /* if .bss is empty move position forward anyways */
                . = ALIGN(4K);
                _ebss = .;
        } > APP 

        ASSERT ((SIZEOF(.bss) <= 4K), "ERROR: .bss region too large, will break PMP compatability.")

        /* Unused space between .bss and .stack
           OFFSET: +0x5000 (20K)
           MAXSIZE: 40K
        */
        .heap : {
                _heap = .;
                . = LENGTH(APP) + ORIGIN(APP) - 4K;
                _eheap = .;
        } > APP

        /* Program stack
           OFFSET: +0xF000 (60K)
           MAXSIZE: 4K
        */
        .stack : {
                _stack = .;
                *(.stack .stack.*)
                _stack_top = .;
        } > APP

        /* End of App (0x80030000) 
           OFFSET: +0x10000 (64K)
        */
	_end = .;
        
        /* Hardcoded since sections are hardcoded */
        __global_pointer$ = _bss;
}
