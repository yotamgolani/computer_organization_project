# triangle.asm
        lw $s0, $zero, $imm, 256    # 0x100 (Address of A)
        lw $s1, $zero, $imm, 257    # 0x101 (Address of B)
        lw $s2, $zero, $imm, 258    # 0x102 (Address of C)
        
        lw $s0, $s0, $zero, 0       # load actual A value
        lw $s1, $s1, $zero, 0       # load actual B value
        lw $s2, $s2, $zero, 0       # load actual C value
        
        # dy = (B - A) / 256
        sub $t0, $s1, $s0, 0        # t0 = B - A
        srl $t0, $t0, $imm, 8       # t0 = dy (number of rows - 1)
        
        # dx = C - B
        sub $t1, $s2, $s1, 0        # t1 = dx (number of cols - 1)
        
        add $s1, $zero, $zero, 0    # s1 = i (row counter, 0 to dy)
        add $gp, $s0, $zero, 0      # gp = current row start address
loopi:
        add $s2, $zero, $zero, 0    # s2 = j (col counter, 0 to dx)
loopj:
        # Check if j*dy <= i*dx
        mul $t2, $s2, $t0, 0        # t2 = j * dy
        mul $a0, $s1, $t1, 0        # a0 = i * dx
        bgt $imm, $t2, $a0, skipp   # if j*dy > i*dx, skip
        
        add $a1, $gp, $s2, 0        # a1 = current row start + j
        out $a1, $imm, $zero, 20    # monitoraddr = a1
        add $a2, $zero, $imm, 255   # a2 = 255 (white)
        out $a2, $imm, $zero, 21    # monitordata = 255
        add $a2, $zero, $imm, 1     # a2 = 1 (write command)
        out $a2, $imm, $zero, 22    # monitorcmd = 1
        
skipp:
        add $s2, $s2, $imm, 1       # j++
        ble $imm, $s2, $t1, loopj    # if j <= dx, repeat
        
        add $s1, $s1, $imm, 1       # i++
        add $gp, $gp, $imm, 256     # move to next row
        ble $imm, $s1, $t0, loopi    # if i <= dy, repeat
        
        halt $zero, $zero, $zero, 0 # halt

.word 256 0x3264
.word 257 0xC864
.word 258 0xC8FA
