.globl app_bin
.globl app_bin_len

.section .rodata.app

app_bin:
	.incbin "app/app_format.bin"
app_bin_len:
	.quad . - app_bin
