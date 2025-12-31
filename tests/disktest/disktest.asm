# disktest.asm
        add $s0, $zero, $imm, 0x300
        add $s2, $zero, $imm, 128
        
        add $t0, $zero, $zero, 0
initloop:
        sw $zero, $s0, $t0, 0
        add $t0, $t0, $imm, 1
        blt $imm, $t0, $s2, initloop

        add $s1, $zero, $zero, 0
sectorloop:
        out $s1, $imm, $zero, 15
        add $t0, $zero, $imm, 0x200
        out $t0, $imm, $zero, 16
        add $t0, $zero, $imm, 1
        out $t0, $imm, $zero, 14
        
waitread:
        in $t0, $imm, $zero, 17
        bne $imm, $t0, $zero, waitread
        
        add $t0, $zero, $zero, 0
accumloop:
        add $t2, $t0, $imm, 0x200
        lw $t1, $t2, $imm, 0
        add $t2, $t0, $imm, 0x300
        lw $a0, $t2, $imm, 0
        add $a0, $a0, $t1, 0
        sw $a0, $t2, $imm, 0
        
        add $t0, $t0, $imm, 1
        blt $imm, $t0, $s2, accumloop
        
        add $s1, $s1, $imm, 1
        add $a1, $zero, $imm, 8
        blt $imm, $s1, $a1, sectorloop
        
        add $t0, $zero, $imm, 8
        out $t0, $imm, $zero, 15
        add $t0, $zero, $imm, 0x300
        out $t0, $imm, $zero, 16
        add $t0, $zero, $imm, 2
        out $t0, $imm, $zero, 14

waitwrite:
        in $t0, $imm, $zero, 17
        bne $imm, $t0, $zero, waitwrite
        
        halt $zero, $zero, $zero, 0
