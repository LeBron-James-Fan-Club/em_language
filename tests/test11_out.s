.text
	.globl main

main:
	push	$fp
	push	$ra
	move	$fp, $sp
	li $t0, 0
	sw	$t0, i
L0:
	lw	$t0, i
	li $t1, 10
	bge	$t0, $t1, L1
	lw	$t0, i
	li	$v0, 1
	move	$a0, $t0
	syscall
	li	$a0, '\n'
	li	$v0, 11
	syscall
	lw	$t0, i
	li $t1, 1
	add	$t1, $t0, $t1
	sw	$t1, i
	b	L0
L1:
	pop	$ra
	pop	$fp
	jr	$ra
	li	$v0, 10
	syscall

.data
	i:	.space 4
