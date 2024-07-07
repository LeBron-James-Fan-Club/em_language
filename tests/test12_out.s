.text
	.globl main

main:
	push	$fp
	push	$ra
	move	$fp, $sp
	li $t0, 72
	sb	$t0, j
	lbu	$t0, j
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 1
	sw	$t0, i
	lw	$t0, i
	li	$v0, 1
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 1
	sw	$t0, i
L0:
	lw	$t0, i
	li $t1, 5
	bgt	$t0, $t1, L1
	lw	$t0, i
	li	$v0, 1
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	lw	$t0, i
	li $t1, 1
	add	$t1, $t0, $t1
	sw	$t1, i
	b	L0
L1:
	li $t0, 253
	sb	$t0, j
L2:
	lbu	$t0, j
	li $t1, 2
	beq	$t0, $t1, L3
	lbu	$t0, j
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	lbu	$t0, j
	li $t1, 1
	add	$t1, $t0, $t1
	sb	$t1, j
	b	L2
L3:
	li $t0, 72
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 101
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 108
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 108
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 111
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 87
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 111
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 114
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 108
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 100
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	pop	$ra
	pop	$fp
	jr	$ra
	li	$v0, 10
	syscall

.data
	i:	.space 4
	j:	.space 1
