.text
	.globl main

main:
	li	$v0, 5
	syscall
	move	$t0, $v0
	sw	$t0, howmany
	lw	$t0, howmany
	li $t1, 0
	bgt	$t0, $t1, L0
	li $t0, 0
	sw	$t0, b
	b	finish
L0:
	lw	$t0, howmany
	li $t1, 1
	bne	$t0, $t1, L1
	li $t0, 1
	sw	$t0, b
	b	finish
L1:
	li $t0, 0
	sw	$t0, i
	li $t0, 0
	sw	$t0, a
	li $t0, 1
	sw	$t0, b
	li $t0, 0
	sw	$t0, c
while:
	lw	$t0, i
	lw	$t1, howmany
	blt	$t0, $t1, L2
	b	finish
L2:
	lw	$t0, a
	lw	$t1, b
	add	$t1, $t0, $t1
	sw	$t1, c
	lw	$t0, b
	sw	$t0, a
	lw	$t0, c
	sw	$t0, b
	lw	$t0, i
	li $t1, 1
	add	$t1, $t0, $t1
	sw	$t1, i
	b	while
finish:
	lw	$t0, b
	li	$v0, 1
	move	$a0, $t0
	syscall
	li	$a0, '\n'
	li	$v0, 11
	syscall
	li	$v0, 10
	syscall

.data
	howmany:	.space 4
	a:	.space 4
	b:	.space 4
	c:	.space 4
	i:	.space 4
