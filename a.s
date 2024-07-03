	.text
main:
	# Locals:
	# - $t0: int i

loop_i_to_10__init:
	li	$t0, 1				# int i = 1;
loop_i_to_10__cond:
	bgt	$t0, 10, loop_i_to_10__end	# if (i > 10) goto loop_i_to_10__end;

loop_i_to_10__body:
	li	$v0, 1				# syscall 1: print_int
	move	$a0, $t0			#
	syscall					# printf("%d", i);

	li	$v0, 11				# syscall 11: print_char
	li	$a0, '\n'			#
	syscall					# putchar('\n');

loop_i_to_10__step:
	addi	$t0, $t0, 1			# i = i + 1;
	b	loop_i_to_10__cond

loop_i_to_10__end:
	li	$v0, 0
	jr	$ra				# return 0;