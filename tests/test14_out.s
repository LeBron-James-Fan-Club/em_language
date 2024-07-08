.text
	.globl main

main:
	push	$fp
	push	$ra
	move	$fp, $sp
	li $t0, 97
	sb	$t0, a
	lbu	$t0, a
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	la	$t0, a
	sw	$t0, b
	lw	$t0, b
	lbu	$t0, 0($t0)
	sb	$t0, c
	lbu	$t0, c
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 12
	sw	$t0, d
	lw	$t0, d
	li	$v0, 1
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	la	$t0, d
	sw	$t0, e
	lw	$t0, e
	lw	$t0, 0($t0)
	sw	$t0, f
	lw	$t0, f
	li	$v0, 1
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
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
	a:	.space 1
	.align 2
	b:	.space 4
	c:	.space 1
	.align 2
	d:	.space 4
	e:	.space 4
	f:	.space 4
