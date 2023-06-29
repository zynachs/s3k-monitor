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
           MAXSIZE: 4K
        */
        .text : { 
                _text = .;
                *(.init)
                *(.text .text.*)
                . = ALIGN(4K);
                _etext = .;
        } > MONITOR

        ASSERT ((SIZEOF(.text) <= 4K), "ERROR: .text region too large, will break PMP compatability.")

        /* Program data 
           OFFSET: +0x1000 (4K) 
           MAXSIZE: 4K
        */
        .data : ALIGN(4K) {
                _data = .;
                *(.data .data.*)
                _sdata = .;
                *(.sdata .sdata.*)
                . = _data == . ? . + 4K : .; /* if .data is empty move position forward anyways */
                . = ALIGN(4K);
                _edata = .;
        } > MONITOR
        
        ASSERT ((SIZEOF(.data) <= 4K), "ERROR: .data region too large, will break PMP compatability.")

        /* Program read-only data
           OFFSET: +0x2000 (8K) 
           MAXSIZE: 32K
        */
        .rodata : ALIGN(4K) {
                _rodata = .;
                *(.rodata .rodata.*)
                _srodata = .;
                *(.srodata .srodata*)
                . = _rodata == . ? . + 32K : .; /* if .rodata is empty move position forward anyways */
                . = ALIGN(32K);
                _erodata = .;
        }

        ASSERT ((SIZEOF(.rodata) <= 32K), "ERROR: .rodata region too large, will break PMP compatability.")

        /* Zero initialized region
           OFFSET: +0x9000 (36K)
           MAXSIZE: 4K
        */
        .bss : ALIGN(4K) {
                _bss = .;
                *(.sbss .sbss.*)
                *(.bss .bss.*)
                . = ALIGN(4K);
                _ebss = .;
        } > MONITOR
        
        ASSERT ((SIZEOF(.bss) <= 4K), "ERROR: .bss region too large, will break PMP compatability.")
        
        /* Unused space between .bss and .stack
           OFFSET: +0xa000 (40K)
           MAXSIZE: 26K
        */
        .heap : {
                _heap = .;
                . = LENGTH(MONITOR) + ORIGIN(MONITOR) - 4K;
                _eheap = .;
        } > MONITOR

        /* Program stack
           OFFSET: +0xF000 (60K)
           MAXSIZE: 4K
        */
        .stack : {
                _stack = .;
                *(.stack .stack.*)
                _stack_top = .;
        } > MONITOR
        
        /* End of monitor (0x80020000) 
           OFFSET: +0x10000 (64K)
        */
	_end = .;

        /* Hardcoded since sections are hardcoded */
        __global_pointer$ = _bss - 12K;
}
