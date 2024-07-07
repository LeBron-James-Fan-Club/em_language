.text
	.globl add

add:
	push	$fp
	push	$ra
	move	$fp, $sp
	li	$v0, 5
	syscall
	move	$t0, $v0
	sw	$t0, a1
	li	$v0, 5
	syscall
	move	$t0, $v0
	sw	$t0, b1
	lw	$t0, a1
	lw	$t1, b1
	add	$t1, $t0, $t1
	sw	$t1, ans1
	lw	$t0, ans1
	move	$v0, $t0
	pop	$ra
	pop	$fp
	jr	$ra
.text
	.globl sub

sub:
	push	$fp
	push	$ra
	move	$fp, $sp
	li	$v0, 5
	syscall
	move	$t0, $v0
	sw	$t0, a2
	li	$v0, 5
	syscall
	move	$t0, $v0
	sw	$t0, b2
	lw	$t0, a2
	lw	$t1, b2
	sub	$t1, $t0, $t1
	sw	$t1, ans2
	lw	$t0, ans2
	move	$v0, $t0
	pop	$ra
	pop	$fp
	jr	$ra
.text
	.globl main

main:
	push	$fp
	push	$ra
	move	$fp, $sp
L0:
	li $t0, 1
	li $t1, 1
	bne	$t0, $t1, L1
	li	$v0, 5
	syscall
	move	$t0, $v0
	sw	$t0, choice
	lw	$t0, choice
	li $t1, 1
	bne	$t0, $t1, L2
	li $t0, 11
	move	$a0, $t0
	jal	add
	move	$t1, $v0
	li	$v0, 1
	move	$a0, $t1
	syscall
	b	L3
L2:
	li $t0, 11
	move	$a0, $t0
	jal	sub
	move	$t1, $v0
	li	$v0, 1
	move	$a0, $t1
	syscall
L3:
	b	L0
L1:
	pop	$ra
	pop	$fp
	jr	$ra
	li	$v0, 10
	syscall

.data
	a1:	.space 4
	b1:	.space 4
	ans1:	.space 4
	a2:	.space 4
	b2:	.space 4
	ans2:	.space 4
	choice:	.space 4
