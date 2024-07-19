.text
	.globl main

main:
	addi $sp, $sp, -12
	sw $ra, 8($sp)
	sw $fp, 4($sp)
	move $fp, $sp
	li $t0, 2
	sw	$t0, -4($fp)
	la	$t0, anon_0
	li	$v0, 4
	move	$a0, $t0
	syscall
	lw	$t0, -4($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 5
	li $t1, 268500992
	sw	$t0, 0($t1)
	li $t0, 268500992
	lw	$t0, 0($t1)
	sw	$t0, -4($fp)
	la	$t0, anon_1
	li	$v0, 4
	move	$a0, $t0
	syscall
	lw	$t0, -4($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 0
	move	$v0, $t0
	b	main_end
main_end:
	lw	$fp, 4($sp)
	lw	$ra, 8($sp)
	addi	$sp, $sp, 12
	jr	$ra
	li	$v0, 10
	syscall

.data
	anon_0:	.asciiz "Joe is "
	anon_1:	.asciiz "Joe is "
