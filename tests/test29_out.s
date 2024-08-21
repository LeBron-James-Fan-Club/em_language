.text
	.globl main

main:
	push $ra
	BEGIN

	addi	$sp, $sp, -8

	li	$v0, 5
	syscall
	move	$t0, $v0
	sw	$t0, 12($sp)

	li	$v0, 5
	syscall
	move	$t0, $v0
	sw	$t0, 16($sp)


	lw	$t0, 12($sp)
	lw	$t1, 16($sp)
	add	$t1, $t0, $t1
	sw	$t1, 12($sp)


	lw	$t0, 12($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall

	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall



	lw	$t0, 12($sp)
	li $t1, 8
	bne	$t0, $t1, L0
	la	$t0, anon_0
	li	$v0, 4
	move	$a0, $t0
	syscall

	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall

	b	L1
L0:
	la	$t0, anon_1
	li	$v0, 4
	move	$a0, $t0
	syscall
L1:


	li $t0, 0
	move	$v0, $t0
	b	main_end

main_end:
	END
	pop	$ra
	jr	$ra

	li	$v0, 10
	syscall

.data
	anon_0:	.asciiz "wow it equals to 8"
	anon_1:	.asciiz "else"
