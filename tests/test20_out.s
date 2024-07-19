.text
	.globl main

main:
	push	$fp
	push	$ra
	move	$fp, $sp
	li $t0, 1
	sw	$t0, bob
	li $t0, 2
	sw	$t0, joe
	li $t0, 8
	sw	$t0, awesome
	la	$t0, anon_0
	li	$v0, 4
	move	$a0, $t0
	syscall
	lw	$t0, bob
	li	$v0, 1
	move	$a0, $t0
	syscall
	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall
	lw	$t0, joe
	li	$v0, 1
	move	$a0, $t0
	syscall
	li $t0, 32
	li	$v0, 11
	move	$a0, $t0
	syscall
	lw	$t0, awesome
	li	$v0, 1
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	la	$t0, anon_1
	li	$v0, 4
	move	$a0, $t0
	syscall
	lw	$t0, bob
	addi	$t0, $t0, 1
	sw	$t0, bob
	li	$v0, 1
	move	$a0, $t0
	syscall
	la	$t0, anon_2
	li	$v0, 4
	move	$a0, $t0
	syscall
	lw	$t0, joe
	move	$t1, $t0
	addi	$t1, $t1, 1
	sw	$t1, joe
	li	$v0, 1
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	la	$t0, anon_3
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
	lw	$t0, awesome
	li $t1, 2
	sllv	$t1, $t0, $t1
	sw	$t1, awesome
	la	$t0, anon_4
	li	$v0, 4
	move	$a0, $t0
	syscall
	lw	$t0, awesome
	li	$v0, 1
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	la	$t0, anon_5
	li	$v0, 4
	move	$a0, $t0
	syscall
	lw	$t0, joe
	addi	$t0, $t0, -1
	sw	$t0, joe
	li	$v0, 1
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	la	$t0, anon_6
	li	$v0, 4
	move	$a0, $t0
	syscall
	lw	$t0, joe
	move	$t1, $t0
	addi	$t1, $t1, -1
	sw	$t1, joe
	li	$v0, 1
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	la	$t0, anon_7
	li	$v0, 4
	move	$a0, $t0
	syscall
	la	$t0, anon_8
	li	$v0, 4
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	la	$t0, anon_9
	li	$v0, 4
	move	$a0, $t0
	syscall
	li	$v0, 5
	syscall
	move	$t0, $v0
	lw	$t1, joe
	sw	$t0, joe
	la	$t0, anon_10
	li	$v0, 4
	move	$a0, $t0
	syscall
	lw	$t0, joe
	li $t1, 1
	and	$t1, $t0, $t1
	li	$v0, 1
	move	$a0, $t1
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	lw	$t0, joe
	li $t1, 1
	and	$t1, $t0, $t1
	beq	$t1, $zero, L0
	la	$t0, anon_11
	li	$v0, 4
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	b	L1
L0:
	la	$t0, anon_12
	li	$v0, 4
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
L1:
	la	$t0, anon_13
	li	$v0, 4
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	la	$t0, anon_14
	li	$v0, 4
	move	$a0, $t0
	syscall
	li	$v0, 5
	syscall
	move	$t0, $v0
	lw	$t1, joe
	sw	$t0, joe
	la	$t0, anon_15
	li	$v0, 4
	move	$a0, $t0
	syscall
	lw	$t0, joe
	li $t1, 1
	or	$t1, $t0, $t1
	li	$v0, 1
	move	$a0, $t1
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	la	$t0, anon_16
	li	$v0, 4
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	la	$t0, anon_17
	li	$v0, 4
	move	$a0, $t0
	syscall
	li	$v0, 5
	syscall
	move	$t0, $v0
	lw	$t1, joe
	sw	$t0, joe
	la	$t0, anon_18
	li	$v0, 4
	move	$a0, $t0
	syscall
	lw	$t0, joe
	li $t1, 1
	xor	$t1, $t0, $t1
	li	$v0, 1
	move	$a0, $t1
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 10
	li	$v0, 11
	move	$a0, $t0
	syscall
	li $t0, 0
	move	$v0, $t0
	pop	$ra
	pop	$fp
	jr	$ra
	li	$v0, 10
	syscall

.data
	bob:	.space 4
	joe:	.space 4
	awesome:	.space 4
	kat:	.space 4
	anon_0:	.asciiz "Values before: "
	anon_1:	.asciiz "Bob: "
	anon_2:	.asciiz " Joe: "
	anon_3:	.asciiz "Joe: "
	anon_4:	.asciiz "Awesome: "
	anon_5:	.asciiz "And his name is JOHN CENA *music plays*: "
	anon_6:	.asciiz "Decrementing after joe mama"
	anon_7:	.asciiz "\n"
	anon_8:	.asciiz "Lets test if the number is odd or even"
	anon_9:	.asciiz "Input number: "
	anon_10:	.asciiz "AND: "
	anon_11:	.asciiz "The number is odd"
	anon_12:	.asciiz "The number is even"
	anon_13:	.asciiz "Bitwise OR test"
	anon_14:	.asciiz "Input number: "
	anon_15:	.asciiz "output: "
	anon_16:	.asciiz "Bitwise XOR test"
	anon_17:	.asciiz "Input number: "
	anon_18:	.asciiz "output: "
