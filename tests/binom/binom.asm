        add $sp, $zero, $imm, 2048  # Set stack pointer to a safe place
        lw $a0, $zero, $imm, 0x100  # Load n from address 0x100
        lw $a1, $zero, $imm, 0x101  # Load k from address 0x101
        jal $ra, $imm, $zero, binom # Call binom(n, k)
        sw $v0, $zero, $imm, 0x102  # Store result at address 0x102
        halt $zero, $zero, $zero, 0 # Halt execution

binom:
        sub $sp, $sp, $imm, 3       # prologue
        sw $ra, $sp, $imm, 0        # save return address
        sw $s0, $sp, $imm, 1        # save $s0
        sw $s1, $sp, $imm, 2        # save $s1

        beq $imm, $a1, $zero, base  # if k == 0, return 1
        beq $imm, $a0, $a1, base    # if n == k, return 1

        add $s0, $a0, $zero, 0      # save n in $s0
        add $s1, $a1, $zero, 0      # save k in $s1

        sub $a0, $s0, $imm, 1       # n = n - 1
        sub $a1, $s1, $imm, 1       # k = k - 1
        jal $ra, $imm, $zero, binom # call binom(n-1, k-1)
        add $s2, $v0, $zero, 0      # save result in $s2 (using $s2 instead of stack to keep it simple, but wait, need to save $s2 too)
                                    # actually, let's use the stack for the first result
        sub $sp, $sp, $imm, 1
        sw $v0, $sp, $imm, 0

        sub $a0, $s0, $imm, 1       # n = n - 1
        add $a1, $s1, $zero, 0      # k = k
        jal $ra, $imm, $zero, binom # call binom(n-1, k)
        
        lw $t0, $sp, $imm, 0        # load binom(n-1, k-1)
        add $sp, $sp, $imm, 1
        add $v0, $v0, $t0, 0        # result = binom(n-1, k) + binom(n-1, k-1)
        
        beq $imm, $zero, $zero, done # unconditional jump to done

base:
        add $v0, $zero, $imm, 1     # return 1

done:
        lw $ra, $sp, $imm, 0        # restore $ra
        lw $s0, $sp, $imm, 1        # restore $s0
        lw $s1, $sp, $imm, 2        # restore $s1
        add $sp, $sp, $imm, 3       # epilogue
        jal $zero, $ra, $zero, 0    # return to caller
