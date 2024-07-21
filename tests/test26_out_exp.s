.text
	.globl deez_nuts


# I push parameters before hand
# and since ra and fp is added to stack it has to be called after

deez_nuts:
	push	$a3
	push	$a2
	push	$a1
	push	$a0
	push $ra
	BEGIN

	lw	$t0, 8($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall

	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 12($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 16($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 20($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, ' '
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 24($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 28($sp)
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
	jr $ra

.text
	.globl main

main:
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
	b	main_end

main_end:
	END
	pop	$ra

	li	$v0, 10
	syscall

.data
