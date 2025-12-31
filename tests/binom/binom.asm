# binom.asm
        add $sp, $zero, $imm, 2048  # Init SP
        lw $a0, $zero, $imm, 0x100  # n
        lw $a1, $zero, $imm, 0x101  # k
        jal $ra, $imm, $zero, binom
        sw $v0, $zero, $imm, 0x102  # Result
        halt $zero, $zero, $zero, 0

binom:
        sub $sp, $sp, $imm, 3
        sw $ra, $sp, $imm, 0
        sw $s0, $sp, $imm, 1
        sw $s1, $sp, $imm, 2

        beq $imm, $a1, $zero, base
        beq $imm, $a0, $a1, base

        add $s0, $a0, $zero, 0
        add $s1, $a1, $zero, 0

        sub $a0, $s0, $imm, 1
        sub $a1, $s1, $imm, 1
        jal $ra, $imm, $zero, binom # binom(n-1, k-1)
        
        # Save result on stack
        sub $sp, $sp, $imm, 1
        sw $v0, $sp, $imm, 0

        sub $a0, $s0, $imm, 1
        add $a1, $s1, $zero, 0
        jal $ra, $imm, $zero, binom # binom(n-1, k)
        
        lw $t0, $sp, $imm, 0
        add $sp, $sp, $imm, 1
        add $v0, $v0, $t0, 0        # result = binom(n-1, k) + binom(n-1, k-1)
        
        beq $imm, $zero, $zero, done

base:
        add $v0, $zero, $imm, 1

done:
        lw $ra, $sp, $imm, 0
        lw $s0, $sp, $imm, 1
        lw $s1, $sp, $imm, 2
        add $sp, $sp, $imm, 3
        jal $zero, $ra, $zero, 0

.word 0x100 5
.word 0x101 2
