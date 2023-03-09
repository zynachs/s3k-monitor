OUTPUT_ARCH("riscv")

ENTRY(_start)

SECTIONS {
	. = 0x80020000;
        .text : {
                _text = .;
                *(.init)
                *(.text .text.*)
                _etext = .;
        }

        .data : {
                _data = .;
                *(.data .data.*)
                *(.rodata .rodata.*)
                _sdata = .;
                *(.sdata .sdata.*)
                *(.srodata .srodata*)
                _edata = .;
        }

        .bss : ALIGN(8) {
                _bss = .;
                *(.sbss .sbss.*)
                *(.bss .bss.*)
		*(.stack .stack.*)
                _ebss = ALIGN(8);
        }

	_end = .;

	. = 0x80020000;
	.noinit : {
		_noinit = .;
                *(.noinit .noinit.*)
                _enoinit = .;
	}

        __global_pointer$ = MIN(_sdata + 0x800, MAX(_data + 0x800, _ebss - 0x800));

}
