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
	li $t1, 5
	bge	$t0, $t1, L1
	lw	$t0, i
	la	$t1, arr
	lw	$t2, i
	sll	$t2, $t2, 2
	add	$t2, $t1, $t2
	sw	$t0, 0($t2)
	lw	$t0, i
	move	$t1, $t0
	addi	$t1, $t1, 1
	sw	$t1, i
	b	L0
L1:
	li	$v0, 5
	syscall
	move	$t0, $v0
	lw	$t1, index
	sw	$t0, index
	la	$t0, anon_0
	li	$v0, 4
	move	$a0, $t0
	syscall
	lw	$t0, index
	li	$v0, 1
	move	$a0, $t0
	syscall
	la	$t0, anon_1
	li	$v0, 4
	move	$a0, $t0
	syscall
	la	$t0, arr
	lw	$t1, index
	sll	$t1, $t1, 2
	add	$t1, $t0, $t1
	lw	$t1, 0($t1)
	li	$v0, 1
	move	$a0, $t1
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
	index:	.space 4
	arr:	.space 20
	i:	.space 4
	anon_0:	.asciiz "Value at index "
	anon_1:	.asciiz " is "
