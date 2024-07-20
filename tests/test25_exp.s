.text
.globl joe

joe:
    addi $sp, $sp, -16
    sw $ra, 12($sp)
    sw $fp, 8($sp)
    move $fp, $sp

    sw $a0, -4($fp)
    sw $a1, -8($fp)
    sw $a2, -12($fp)
    sw $a3, -16($fp)

    addi $sp, $sp, -16

    lw $t0, -4($fp)
    li $v0, 1
    move $a0, $t0
    syscall

    lw $t0, -8($fp)
    li $v0, 1
    move $a0, $t0
    syscall

    lw $t0, -12($fp)
    li $v0, 1
    move $a0, $t0
    syscall

    lw $t0, -16($fp)
    li $v0, 1
    move $a0, $t0
    syscall

    li $t0, 10
    li $v0, 11
    move $a0, $t0
    syscall

    lw $fp, 8($sp)
    lw $ra, 12($sp)
    addi $sp, $sp, 16
    jr $ra

.text
.globl yuck

yuck:
    addi $sp, $sp, -16
    sw $ra, 12($sp)
    sw $fp, 8($sp)
    move $fp, $sp

    sw $a0, -4($fp)
    sw $a1, -8($fp)

    lw $t0, -4($fp)
    li $v0, 1
    move $a0, $t0
    syscall

    lw $t0, -8($fp)
    li $v0, 1
    move $a0, $t0
    syscall

    li $t0, 10
    li $v0, 11
    move $a0, $t0
    syscall

    lw $fp, 8($sp)
    lw $ra, 12($sp)
    addi $sp, $sp, 16
    jr $ra

.text
.globl choo_choo

choo_choo:
    addi $sp, $sp, -20
    sw $ra, 16($sp)
    sw $fp, 12($sp)
    move $fp, $sp

    sw $a0, -4($fp)
    sw $a1, -8($fp)
    sw $a2, -12($fp)
    sw $a3, -16($fp)

    lw $t0, -4($fp)
    li $v0, 1
    move $a0, $t0
    syscall

    lw $t0, -8($fp)
    li $v0, 1
    move $a0, $t0
    syscall

    lw $t0, -12($fp)
    li $v0, 1
    move $a0, $t0
    syscall

    lw $t0, -16($fp)
    li $v0, 1
    move $a0, $t0
    syscall

    li $t0, 10
    li $v0, 11
    move $a0, $t0
    syscall

    lw $fp, 12($sp)
    lw $ra, 16($sp)
    addi $sp, $sp, 20
    jr $ra

.text
.globl deez_nuts

deez_nuts:
    addi $sp, $sp, -16
    sw $ra, 12($sp)
    sw $fp, 8($sp)
    move $fp, $sp
    addi $sp, $sp, -24

    li $v0, 5
    syscall
    move $t0, $v0
    sw $t0, -4($fp)

    li $v0, 5
    syscall
    move $t0, $v0
    sw $t0, -8($fp)

    li $v0, 5
    syscall
    move $t0, $v0
    sw $t0, -12($fp)

    li $v0, 5
    syscall
    move $t0, $v0
    sw $t0, -16($fp)

    li $v0, 5
    syscall
    move $t0, $v0
    sw $t0, -20($fp)

    li $v0, 5
    syscall
    move $t0, $v0
    sw $t0, -24($fp)

    lw $t0, -4($fp)
    li $v0, 1
    move $a0, $t0
    syscall

    lw $t0, -8($fp)
    li $v0, 1
    move $a0, $t0
    syscall

    lw $t0, -12($fp)
    li $v0, 1
    move $a0, $t0
    syscall

    lw $t0, -16($fp)
    li $v0, 1
    move $a0, $t0
    syscall

    lw $t0, -20($fp)
    li $v0, 1
    move $a0, $t0
    syscall

    lw $t0, -24($fp)
    li $v0, 1
    move $a0, $t0
    syscall

    li $t0, 10
    li $v0, 11
    move $a0, $t0
    syscall

    lw $fp, 8($sp)
    lw $ra, 12($sp)
    addi $sp, $sp, 40
    jr $ra

.text
.globl main

main:
    addi $sp, $sp, -8
    sw $ra, 4($sp)
    sw $fp, 8($sp)
    move $fp, $sp

    li $a0, 1
    li $a1, 2
    jal yuck

    li $t0, 26
    sw $t0, 0($sp)
    li $t0, 25
    sw $t0, -4($sp)
    li $t0, 24
    sw $t0, -8($sp)
    li $t0, 23
    sw $t0, -12($sp)
    li $t0, 22
    sw $t0, -16($sp)
    li $t0, 21
    sw $t0, -20($sp)
    li $t0, 20
    sw $t0, -24($sp)
    li $t0, 19
    sw $t0, -28($sp)
    li $t0, 18
    sw $t0, -32($sp)
    li $t0, 17
    sw $t0, -36($sp)
    li $t0, 16
    sw $t0, -40($sp)
    li $t0, 15
    sw $t0, -44($sp)
    li $t0, 14
    sw $t0, -48($sp)
    li $t0, 13
    sw $t0, -52($sp)
    li $t0, 12
    sw $t0, -56($sp)
    li $t0, 11
    sw $t0, -60($sp)
    li $t0, 10
    sw $t0, -64($sp)
    li $t0, 9
    sw $t0, -68($sp)
    li $t0, 8
    sw $t0, -72($sp)
    li $t0, 7
    sw $t0, -76($sp)
    li $t0, 6
    sw $t0, -80($sp)
    li $t0, 5
    sw $t0, -84($sp)
    li $t0, 4
    move $a3, $t0
    li $t0, 3
    move $a2, $t0
    li $t0, 2
    move $a1, $t0
    li $t0, 1
    move $a0, $t0
    jal choo_choo

    lw $fp, 8($sp)
    lw $ra, 4($sp)
    addi $sp, $sp, 8
    jr $ra

    li $v0, 10
    syscall

.data
