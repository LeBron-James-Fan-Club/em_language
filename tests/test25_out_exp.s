.text
	.globl joe

joe:
	push	$a3
	push	$a2
	push	$a1
	push	$a0
	push $ra
	BEGIN


	lw	$t0, 8($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall

	lw	$t0, 12($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, 16($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, 20($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, 24($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, 28($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, 32($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, 36($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, 40($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall


	li $t0, 0
	move	$v0, $t0
	b	joe_end

joe_end:
	END
	pop	$ra
	jr	$ra

.text
	.globl yuck

yuck:
	push	$a1
	push	$a0
	push $ra
	BEGIN


	lw	$t0, 8($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall

	lw	$t0, 12($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall


	li $t0, 0
	move	$v0, $t0
	b	yuck_end

yuck_end:
	END
	pop	$ra
	jr	$ra

.text
	.globl choo_choo

choo_choo:
	push	$a3
	push	$a2
	push	$a1
	push	$a0
	push $ra
	BEGIN

	addi	$sp, $sp, -4

	lw	$t0, 8($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall

	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 12($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


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


	lw	$t0, 40($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 44($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 48($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 52($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 56($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 60($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 64($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 68($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 72($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 76($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 80($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 84($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 88($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 92($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 96($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 100($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 104($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, 108($fp)
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


	li $t0, 69
	sw	$t0, 120($fp)


	lw	$t0, 120($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 0
	move	$v0, $t0
	b	choo_choo_end

choo_choo_end:
	END
	pop	$ra
	jr	$ra

.text
	.globl deez_nuts

deez_nuts:
	push $ra
	BEGIN

	addi	$sp, $sp, -24

	li	$v0, 5
	syscall
	move	$t0, $v0
	lw	$t1, -4($fp)
	sw	$t0, -4($sp)

	li	$v0, 5
	syscall
	move	$t0, $v0
	lw	$t1, -8($fp)
	sw	$t0, -8($sp)


	li	$v0, 5
	syscall
	move	$t0, $v0
	lw	$t1, -12($fp)
	sw	$t0, -12($sp)


	li	$v0, 5
	syscall
	move	$t0, $v0
	lw	$t1, -16($fp)
	sw	$t0, -16($sp)


	li	$v0, 5
	syscall
	move	$t0, $v0
	lw	$t1, -20($fp)
	sw	$t0, -20($sp)


	li	$v0, 5
	syscall
	move	$t0, $v0
	lw	$t1, -24($fp)
	sw	$t0, -24($sp)


	lw	$t0, -4($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall

	lw	$t0, -8($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, -12($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, -16($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, -20($fp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, -24($fp)
	li	$v0, 1
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


	li $t0, 2
	move	$a1, $t0
	li $t0, 1
	move	$a0, $t0
	jal	yuck
	move	$t0, $v0

	li $t0, 26
	push	$t0
	li $t0, 25
	push	$t0
	li $t0, 24
	push	$t0
	li $t0, 23
	push	$t0
	li $t0, 22
	push	$t0
	li $t0, 21
	push	$t0
	li $t0, 20
	push	$t0
	li $t0, 19
	push	$t0
	li $t0, 18
	push	$t0
	li $t0, 17
	push	$t0
	li $t0, 16
	push	$t0
	li $t0, 15
	push	$t0
	li $t0, 14
	push	$t0
	li $t0, 13
	push	$t0
	li $t0, 12
	push	$t0
	li $t0, 11
	push	$t0
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
	jal	choo_choo
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
