.text
	.globl bob

bob:
	addi $sp, $sp, -16
	sw $ra, 12($sp)
	sw $fp, 8($sp)
	move $fp, $sp
	li $t0, 2
	sw	$t0, -4($fp)
	li $t0, 3
	sw	$t0, -8($fp)
	la	$t0, anon_0
	li	$v0, 4
	move	$a0, $t0
	syscall
	lw	$t0, -8($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	lw	$t0, -4($sp)
	move	$v0, $t0
	b	bob_end
bob_end:
	lw	$fp, 8($sp)
	lw	$ra, 12($sp)
	addi	$sp, $sp, 16
	jr	$ra
.text
	.globl main

main:
	addi $sp, $sp, -8
	sw $ra, 4($sp)
	sw $fp, 0($sp)
	move $fp, $sp
	la	$t0, anon_1
	li	$v0, 4
	move	$a0, $t0
	syscall
	lw	$t0, joe
	li	$v0, 1
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 2
	move	$a0, $t0
	jal	bob
	move	$t1, $v0
	li $t0, 0
	move	$v0, $t0
	b	main_end
main_end:
	lw	$fp, 0($sp)
	lw	$ra, 4($sp)
	addi	$sp, $sp, 8
	jr	$ra
	li	$v0, 10
	syscall

.data
	joe:	.word 2
	anon_0:	.asciiz "yum: "
	anon_1:	.asciiz "Joe is "
