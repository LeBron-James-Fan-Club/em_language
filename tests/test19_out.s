.text
	.globl main

main:
	push	$fp
	push	$ra
	move	$fp, $sp
	la	$t0, anon_0
	sw	$t0, p
	li $t0, 2
	sw	$t0, a
	lw	$t0, p
	li	$v0, 4
	move	$a0, $t0
	syscall
	lw	$t0, a
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
	p:	.space 4
	a:	.space 4
	anon_0:	.asciiz "THE ONE PIECE, THE ONE PIECE IS REAL\n- Whitebeard\n"
