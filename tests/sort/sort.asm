# sort.asm
        add $s0, $zero, $imm, 16
outer:
        sub $s0, $s0, $imm, 1
        beq $imm, $s0, $zero, end
        add $s1, $zero, $zero, 0
inner:
        add $t0, $s1, $imm, 0x100
        lw $t1, $t0, $imm, 0
        lw $t2, $t0, $imm, 1
        
        ble $imm, $t1, $t2, skip
        sw $t2, $t0, $imm, 0
        sw $t1, $t0, $imm, 1
skip:
        add $s1, $s1, $imm, 1
        blt $imm, $s1, $s0, inner
        beq $imm, $zero, $zero, outer
end:
        halt $zero, $zero, $zero, 0

.word 0x100 12
.word 0x101 5
.word 0x102 8
.word 0x103 1
.word 0x104 9
.word 0x105 14
.word 0x106 3
.word 0x107 10
.word 0x108 0
.word 0x109 15
.word 0x10A 2
.word 0x10B 7
.word 0x10C 6
.word 0x10D 13
.word 0x10E 4
.word 0x10F 11
