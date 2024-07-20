.text
	.globl joe

joe:
	addi $sp, $sp, -24
	sw $ra, 20($sp)
	sw $fp, 16($sp)
	move $fp, $sp
	sw	$a0, -4($fp)
	sw	$a1, -8($fp)
	sw	$a2, -12($fp)
	sw	$a3, -16($fp)
	lw	$t0, -4($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall

	lw	$t0, -8($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, -12($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, -16($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, -20($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, -24($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, -28($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, -32($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, -36($sp)
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
	lw	$fp, 16($sp)
	lw	$ra, 20($sp)
	addi	$sp, $sp, 24
	jr	$ra
.text
	.globl yuck

yuck:
	addi $sp, $sp, -16
	sw $ra, 12($sp)
	sw $fp, 8($sp)
	move $fp, $sp
	sw	$a0, -4($fp)
	sw	$a1, -8($fp)
	lw	$t0, -4($sp)
	li	$v0, 1
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


	li $t0, 0
	move	$v0, $t0
	b	yuck_end

yuck_end:
	lw	$fp, 8($sp)
	lw	$ra, 12($sp)
	addi	$sp, $sp, 16
	jr	$ra
.text
	.globl choo_choo

choo_choo:
	addi $sp, $sp, -28
	sw $ra, 24($sp)
	sw $fp, 20($sp)
	move $fp, $sp
	
    sw	$a0, -4($fp)
	sw	$a1, -8($fp)
	sw	$a2, -12($fp)
	sw	$a3, -16($fp)

	lw	$t0, -4($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall

	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -8($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -12($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -16($sp)
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


	lw	$t0, -24($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -28($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -32($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -36($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -40($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -44($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -48($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -52($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -56($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -60($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -64($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -68($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -72($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -76($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -80($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -84($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -88($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -92($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -96($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -100($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall


	lw	$t0, -104($sp)
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
	sw	$t0, -20($fp)


	lw	$t0, -20($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	li $t0, 0
	move	$v0, $t0
	b	choo_choo_end

choo_choo_end:
	lw	$fp, 20($sp)
	lw	$ra, 24($sp)
	addi	$sp, $sp, 28
	jr	$ra
.text
	.globl deez_nuts

deez_nuts:
	addi $sp, $sp, -32
	sw $ra, 28($sp)
	sw $fp, 24($sp)
	move $fp, $sp
	li	$v0, 5
	syscall
	move	$t0, $v0
	lw	$t1, -4($sp)
	sw	$t0, -4($fp)

	li	$v0, 5
	syscall
	move	$t0, $v0
	lw	$t1, -8($sp)
	sw	$t0, -8($fp)


	li	$v0, 5
	syscall
	move	$t0, $v0
	lw	$t1, -12($sp)
	sw	$t0, -12($fp)


	li	$v0, 5
	syscall
	move	$t0, $v0
	lw	$t1, -16($sp)
	sw	$t0, -16($fp)


	li	$v0, 5
	syscall
	move	$t0, $v0
	lw	$t1, -20($sp)
	sw	$t0, -20($fp)


	li	$v0, 5
	syscall
	move	$t0, $v0
	lw	$t1, -24($sp)
	sw	$t0, -24($fp)


	lw	$t0, -4($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall

	lw	$t0, -8($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, -12($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, -16($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, -20($sp)
	li	$v0, 1
	move	$a0, $t0
	syscall


	lw	$t0, -24($sp)
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
	lw	$fp, 24($sp)
	lw	$ra, 28($sp)
	addi	$sp, $sp, 32
	jr	$ra
.text
	.globl main

main:
	addi $sp, $sp, -8
	sw $ra, 4($sp)
	sw $fp, 0($sp)
	move $fp, $sp
	li $t0, 2
	move	$a1, $t0
	li $t0, 1
	move	$a0, $t0
	jal	yuck
	move	$t0, $v0

	addi	$sp, $sp, -88
	li $t0, 26
	sw	$t0, -84($sp)
	li $t0, 25
	sw	$t0, -80($sp)
	li $t0, 24
	sw	$t0, -76($sp)
	li $t0, 23
	sw	$t0, -72($sp)
	li $t0, 22
	sw	$t0, -68($sp)
	li $t0, 21
	sw	$t0, -64($sp)
	li $t0, 20
	sw	$t0, -60($sp)
	li $t0, 19
	sw	$t0, -56($sp)
	li $t0, 18
	sw	$t0, -52($sp)
	li $t0, 17
	sw	$t0, -48($sp)
	li $t0, 16
	sw	$t0, -44($sp)
	li $t0, 15
	sw	$t0, -40($sp)
	li $t0, 14
	sw	$t0, -36($sp)
	li $t0, 13
	sw	$t0, -32($sp)
	li $t0, 12
	sw	$t0, -28($sp)
	li $t0, 11
	sw	$t0, -24($sp)
	li $t0, 10
	sw	$t0, -20($sp)
	li $t0, 9
	sw	$t0, -16($sp)
	li $t0, 8
	sw	$t0, -12($sp)
	li $t0, 7
	sw	$t0, -8($sp)
	li $t0, 6
	sw	$t0, -4($sp)
	li $t0, 5
	sw	$t0, 0($sp)
	li $t0, 4
	move	$a3, $t0
	li $t0, 3
	move	$a2, $t0
	li $t0, 2
	move	$a1, $t0
	li $t0, 1
	move	$a0, $t0
	jal	choo_choo
	addi	$sp, $sp, 88
	move	$t0, $v0


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
