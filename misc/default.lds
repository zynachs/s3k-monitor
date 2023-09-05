/* Optimized linker script used for MONITOR, not compatible with PMP */

OUTPUT_ARCH("riscv")

ENTRY(_start)

_uart = 0x10000000;
PROVIDE(_uart = 0x10000000);

SECTIONS {
        .text : ALIGN(4K) {
                _text = .;
                *(.init)
                *(.text .text.*)
                _etext = .;
        }

        .data : ALIGN(8) {
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
                _ebss = ALIGN(8);
        }

	.stack : ALIGN(8) {
		*(.stack .stack.*)
	}
	_end = .;

        __global_pointer$ = MIN(_sdata + 0x800, MAX(_data + 0x800, _ebss - 0x800));

}
