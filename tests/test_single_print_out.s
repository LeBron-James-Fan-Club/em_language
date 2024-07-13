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
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 97
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 12
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 97
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 0
	li	$v0, 5
	syscall
	move	$t1, $v0
	move	$v0, $t1
	pop	$ra
	pop	$fp
	jr	$ra
	li	$v0, 10
	syscall

.data
	anon_0:	.asciiz "Hello, World!\n"
