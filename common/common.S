.globl memset
.type memset,@function

memset:
	mv	t0,a0
	add	t1,a0,a2
	j	2f
1:	sb	a1,(t0)
	addi	t0,t0,1
2:	bne	t0,t1,1b
	ret

.globl memcpy
.type memcpy,@function
memcpy:
	mv	t0,a0
	add	a2,a2,a1
	j	2f
1:	lb	t2,(a1)
	addi	a1,a1,1
	sb	t2,(t0)
	addi	t0,t0,1
2:	bne	a1,a2,1b
	ret
