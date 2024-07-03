	.text
	.globl main

main:
	li $t0, 3
	li $t1, 12
	mul	$t0, $t1, $t0
	li	$v0, 1
	move	$a0, $t0
	syscall
	li	$a0, '\n'
	li	$v0, 11
	syscall
	li $t0, 4
	li $t1, 2
	mul	$t0, $t1, $t0
	li $t1, 18
	sub	$t0, $t1, $t0
	li	$v0, 1
	move	$a0, $t0
	syscall
	li	$a0, '\n'
	li	$v0, 11
	syscall
	li $t0, 5
	li $t1, 3
	mul	$t0, $t1, $t0
	li $t1, 2
	li $t2, 5
	div	$t1, $t2, $t1
	li $t2, 9
	sub	$t1, $t2, $t1
	add	$t0, $t1, $t0
	li $t1, 2
	add	$t0, $t1, $t0
	li $t1, 1
	add	$t0, $t1, $t0
	li	$v0, 1
	move	$a0, $t0
	syscall
	li	$a0, '\n'
	li	$v0, 11
	syscall
	li	$v0, 10
	syscall
