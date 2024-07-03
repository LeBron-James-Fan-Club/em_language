	.text
	.globl main

main:
	li $t0, 2
	li $t1, 2
	li $t2, 2
	mul	$t2, $t1, $t2
	add	$t2, $t0, $t2
	li	$v0, 1
	mov	$a0, $t2
	syscall
	li	$v0, 10
	syscall
