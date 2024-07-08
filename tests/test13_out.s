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
