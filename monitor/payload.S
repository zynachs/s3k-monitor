.globl uart_bin
.globl uart_bin_len
.globl app_bin
.globl app_bin_len


.section .rodata.payloads

uart_bin:
	.incbin "../uart/uart.bin"
uart_bin_len:
.quad . - uart_bin

app_bin:
	.incbin "../app/app.bin"
app_bin_len:
.quad . - app0_bin

