/* Linker script which links a binary compatible with PMP intended for APP (incredibly bloated and non-optimized) */

OUTPUT_ARCH("riscv")

ENTRY(_start)

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
        }

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
                . = _data == . ? . + 8K : .; /* if .data is empty move position forward anyways */
                . = ALIGN(8K);
                _edata = .;
        }

        ASSERT ((SIZEOF(.data) <= 8K), "ERROR: .data region too large, will break PMP compatability.")

        /* Program read-only data
           OFFSET: +0x4000 (16K) 
           MAXSIZE: 8K
        */
        .rodata : ALIGN(8K) {
                _rodata = .;
                *(.rodata .rodata.*)
                _srodata = .;
                *(.srodata .srodata*)
                . = _rodata == . ? . + 8K : .; /* if .rodata is empty move position forward anyways */
                . = ALIGN(8K);
                _erodata = .;
        }
        
        ASSERT ((SIZEOF(.rodata) <= 8K), "ERROR: .rodata region too large, will break PMP compatability.")
        
        /* Zero initialized region
           OFFSET: +0x6000 (24K)
           MAXSIZE: 8K
        */
        .bss : ALIGN(8K) {
                _bss = .;
                *(.sbss .sbss.*)
                *(.bss .bss.*)
                . = _bss == . ? . + 8K : .; /* if .bss is empty move position forward anyways */
                . = ALIGN(8K);
                _ebss = .;
        }

        ASSERT ((SIZEOF(.bss) <= 8K), "ERROR: .bss region too large, will break PMP compatability.")

        /* Unused space between .bss and .stack
           OFFSET: +0x8000 (32K)
           MAXSIZE: 28K
        */
        .heap : ALIGN(8K) {
                _heap = .;
                . = . + 28K;
                _eheap = .;
        }

        /* Program stack
           OFFSET: +0xF000 (60K)
           MAXSIZE: 4K
        */
        .stack : {
                _stack = .;
                *(.stack .stack.*)
                _stack_top = .;
        }

        /* End of App (0x80030000) 
           OFFSET: +0x10000 (64K)
        */
	_end = .;
        
        /* Hardcoded since sections are hardcoded */
        __global_pointer$ = _bss;
}
