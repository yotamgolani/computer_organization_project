        add $s0, $zero, $imm, 16    # s0 = 16 (number of elements)
outer:
        sub $s0, $s0, $imm, 1       # s0--
        beq $imm, $s0, $zero, end   # if s0 == 0, we are done
        add $s1, $zero, $zero, 0    # s1 = 0 (inner loop index j)
inner:
        add $t0, $s1, $imm, 0x100   # t0 = 0x100 + j (address of current element)
        lw $t1, $t0, $imm, 0        # t1 = MEM[0x100 + j]
        lw $t2, $t0, $imm, 1        # t2 = MEM[0x100 + j + 1]
        
        ble $imm, $t1, $t2, skip    # if MEM[j] <= MEM[j+1], skip swap
        sw $t2, $t0, $imm, 0        # MEM[j] = t2
        sw $t1, $t0, $imm, 1        # MEM[j+1] = t1
skip:
        add $s1, $s1, $imm, 1       # j++
        blt $imm, $s1, $s0, inner   # if j < s0, repeat inner loop
        beq $imm, $zero, $zero, outer # repeat outer loop
end:
        halt $zero, $zero, $zero, 0 # halt execution
