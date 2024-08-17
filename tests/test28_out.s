.text
	.globl deez_nuts

deez_nuts:
	push	$a3
	push	$a2
	push	$a1
	push	$a0
	push $ra
	BEGIN

	addi	$sp, $sp, -24

	lw	$t0, 32($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall

	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 36($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 40($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 44($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 48($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 52($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall


	li $t0, 1
	move	$v0, $t0
	b	deez_nuts_end

deez_nuts_end:
	END
	pop	$ra
	jr	$ra

.text
	.globl main

main:
	push $ra
	BEGIN

	addi	$sp, $sp, -4

	li $t0, 0
	sw	$t0, 12($sp)

	lw	$t0, 12($sp)
	li $t1, 2
	add	$t1, $t0, $t1
	sw	$t1, 12($sp)


	lw	$t0, 12($sp)
	li $t1, 2
	mul	$t1, $t0, $t1
	sw	$t1, 12($sp)


	lw	$t0, 12($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall

	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall



	li $t0, 6
	push	$t0
	li $t0, 5
	push	$t0
	li $t0, 4
	move	$a3, $t0
	li $t0, 3
	move	$a2, $t0
	li $t0, 2
	move	$a1, $t0
	li $t0, 1
	move	$a0, $t0
	jal	deez_nuts
	move	$t0, $v0


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
