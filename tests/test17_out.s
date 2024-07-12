.text
	.globl main

main:
	push	$fp
	push	$ra
	move	$fp, $sp
	li $t0, 12
	la	$t1, b
	li $t2, 3
	sll	$t2, $t2, 2
	add	$t2, $t1, $t2
	sw	$t0, 0($t2)
	la	$t0, b
	li $t1, 3
	sll	$t1, $t1, 2
	add	$t1, $t0, $t1
	lw	$t1, 0($t1)
	sw	$t1, a
	lw	$t0, a
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
	a:	.space 4
	b:	.space 100
