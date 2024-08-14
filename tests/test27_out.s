.text
	.globl deez_nuts

deez_nuts:
	push	$a3
	push	$a2
	push	$a1
	push	$a0
	push $ra
	BEGIN


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
	b	deez_nuts_end

deez_nuts_end:
	END
	pop	$ra
	jr	$ra

.text
	.globl deez_nuts

deez_nuts:
	push $ra
	BEGIN

	addi	$sp, $sp, -24

	lw	$t0, 16($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall

	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 20($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 24($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 28($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 32($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 36($fp)
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

	li	$v0, 10
	syscall

.data
