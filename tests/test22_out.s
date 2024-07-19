.text
	.globl main

main:
	push	$fp
	push	$ra
	move	$fp, $sp
	la	$t0, anon_0
	li	$v0, 4
	move	$a0, $t0
	syscall
	lw	$t0, joe
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
	joe:	.word 2
	anon_0:	.asciiz "Joe is "
