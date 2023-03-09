.globl _start
.type _start, @function

.section .init
_start:

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
.fill 2048
_stack_top:
