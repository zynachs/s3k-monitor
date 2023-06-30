.globl _start # make _start available to linker
.type _start, @function

.section .init # following code to go into section named .init. (apparently good practice if running within a OS)
_start: # start of symbol _start, following code defines the symbol

.option push
.option norelax
	lla	gp,__global_pointer$
.option pop
	lla	sp,_stack_top

	lla	t0,_bss
	lla	t1,_ebss
	j	2f
1:	sd	zero,(t0)
	addi	t0,t0,8
2:	bne	t0,t1,1b
	
	call	setup
	lla	ra,loop
	tail	loop

.section .stack
_stack:
.fill 4096
_stack_top:
