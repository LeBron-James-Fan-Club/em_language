.text
	.globl main

main:
	push	$fp
	push	$ra
	move	$fp, $sp
	la	$t0, b
	sw	$t0, z
	li $t0, 42
	sw	$t0, b
	li $t0, 2
	sw	$t0, c
	lw	$t0, z
	lw	$t0, 0($t0)
	li	$v0, 1
	move	$a0, $t0
	syscall
	lw	$t0, c
	li	$v0, 1
	move	$a0, $t0
	syscall
	li $t0, 0
	move	$v0, $t0
	pop	$ra
	pop	$fp
	jr	$ra
	li	$v0, 10
	syscall

.data
	b:	.space 4
	c:	.space 4
	z:	.space 4
