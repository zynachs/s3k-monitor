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
           MAXsize: 4K
        */
        .text : { 
                _text = .;
                *(.init)
                *(.init.trap)
                *(.text .text.*)
                _etext = .;
        } > APP

        ASSERT ((SIZEOF(.text) <= 4K), "ERROR: .text region too large, will break PMP compatability.")

        /* Program data 
           OFFSET: +0x1000 (4K) 
           MAXSIZE: 2K
        */
        .data : ALIGN(4K) {
                _data = .;
                *(.data .data.*)
                _sdata = .;
                *(.sdata .sdata.*)
                _edata = .;
                . = _data == . ? . + 2K : .; /* if .data is empty move position forward anyways */
        } > APP

        ASSERT ((SIZEOF(.data) <= 2K), "ERROR: .data region too large, will break PMP compatability.")

        /* Program read-only data
           OFFSET: +0x1800 (2K) 
           MAXSIZE: 2K
        */
        .rodata : ALIGN(2K) {
                _rodata = .;
                *(.rodata .rodata.*)
                _srodata = .;
                *(.srodata .srodata*)
                _erodata = .;
                . = _rodata == . ? . + 2K : .; /* if .rodata is empty move position forward anyways */
        } > APP
        
        ASSERT ((SIZEOF(.rodata) <= 2K), "ERROR: .rodata region too large, will break PMP compatability.")
        
        /* Zero initialized region
           OFFSET: +0x2000 (8K)
           MAXSIZE: 4K
        */
        .bss : ALIGN(4K) {
                _bss = .;
                *(.sbss .sbss.*)
                *(.bss .bss.*)
                _ebss = ALIGN(8);
        } > APP

        ASSERT ((SIZEOF(.bss) <= 4K), "ERROR: .bss region too large, will break PMP compatability.")

        /* Program stack
           OFFSET: +0xF800 (62K)
           MAXSIZE: 4K
        */
        .stack : {
                . = (ORIGIN(APP) + LENGTH(APP)) - 4K;
                /* Dedicated stack for trap_handler */
                _trapstack = .;
                . += 2K;
                _trapstack_top = .;
                /* Generic stack */
                *(.stack)
        } > APP

        /* Hardcoded to start of .bss since it in the middle between .data and .bss */
        __global_pointer$ = _bss;
        
        /* End of App (0x80030000) 
           OFFSET: +0x10000 (64K)
        */
	_end = .;
}
