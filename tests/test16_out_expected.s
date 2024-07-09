.text
	.globl main

main:
	push	$fp
	push	$ra
	move	$fp, $sp
	li $t0, 12
	sw	$t0, c
	li $t0, 18
	sw	$t0, d
	lw	$t0, c
	li	$v0, 1
	move	$a0, $t0
	syscall
	la	$t0, c

	li $t1, 1
	sll	$t1, $t1, 2
	add	$t1, $t0, $t1
	
	sw	$t1, e
	lw	$t0, e
	lw	$t0, 0($t0)
	sw	$t0, f
	lw	$t0, f
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
	c:	.space 4
	d:	.space 4
	e:	.space 4
	f:	.space 4
