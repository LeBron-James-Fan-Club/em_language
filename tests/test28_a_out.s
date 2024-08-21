.text
	.globl deez_nuts

deez_nuts:
	push	$a3
	push	$a2
	push	$a1
	push	$a0
	BEGIN

	push $ra
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
	pop	$ra
	END
	jr	$ra

.text
	.globl main

main:
	BEGIN

	push $ra
	addi	$sp, $sp, -4

	li $t0, 0
	sw	$t0, 12($sp)

	lw	$t0, 12($sp)
	li $t1, 2
	add	$t1, $t0, $t1
	sw	$t1, 12($sp)


	lw	$t0, 12($sp)
	li $t1, 2
	li $t2, 2
	add	$t2, $t1, $t2
	mul	$t2, $t0, $t2
	li $t0, 2
	mul	$t0, $t2, $t0
	sw	$t0, 12($sp)


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


	li $t0, 10
	push	$t0
	li $t0, 9
	push	$t0
	li $t0, 8
	push	$t0
	li $t0, 7
	push	$t0
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
	jal	another
	move	$t0, $v0


	li $t0, 0
	move	$v0, $t0
	b	main_end

main_end:
	pop	$ra
	END
	jr	$ra

.text
	.globl another

another:
	push	$a3
	push	$a2
	push	$a1
	push	$a0
	BEGIN

	push $ra
	addi	$sp, $sp, -40

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


	lw	$t0, 56($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 60($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 64($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 68($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 72($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 76($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 80($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 84($sp)
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
	b	another_end

another_end:
	pop	$ra
	END
	jr	$ra

	li	$v0, 10
	syscall
