lw $s0, $zero, $imm, 412    # s0 = pixel address of A (0x19C) - wait, let's use the .word values
        lw $s0, $zero, $imm, 256    # 0x100
        lw $s1, $zero, $imm, 257    # 0x101
        lw $s2, $zero, $imm, 258    # 0x102
        
        lw $s0, $s0, $zero, 0       # load actual A value
        lw $s1, $s1, $zero, 0       # load actual B value
        lw $s2, $s2, $zero, 0       # load actual C value
        
        # dy = (B - A) / 256
        sub $t0, $s1, $s0, 0        # t0 = B - A
        srl $t0, $t0, $imm, 8       # t0 = dy (number of rows - 1)
        
        # dx = C - B
        sub $t1, $s2, $s1, 0        # t1 = dx (number of cols - 1)
        
        add $s1, $zero, $zero, 0    # s1 = i (row counter, 0 to dy)
        add $s3, $s0, $zero, 0      # s3 = current row start address
loop_i:
        add $s2, $zero, $zero, 0    # s2 = j (col counter, 0 to dx)
loop_j:
        # Check if j*dy <= i*dx (this is the condition for being inside the triangle ABC)
        mul $t2, $s2, $t0, 0        # t2 = j * dy
        mul $t3, $s1, $t1, 0        # t3 = i * dx
        bgt $imm, $t2, $t3, skip_p  # if j*dy > i*dx, the pixel is outside the triangle
        
        add $t4, $s3, $s2, 0        # t4 = current row start + j (pixel address)
        out $t4, $imm, $zero, 20    # monitoraddr = t4
        add $t5, $zero, $imm, 255   # t5 = 255 (white)
        out $t5, $imm, $zero, 21    # monitordata = 255
        add $t5, $zero, $imm, 1     # t5 = 1 (write command)
        out $t5, $imm, $zero, 22    # monitorcmd = 1
        
skip_p:
        add $s2, $s2, $imm, 1       # j++
        ble $imm, $s2, $t1, loop_j   # if j <= dx, repeat inner loop
        
        add $s1, $s1, $imm, 1       # i++
        add $s3, $s3, $imm, 256     # move to next row (256 pixels per row)
        ble $imm, $s1, $t0, loop_i   # if i <= dy, repeat outer loop
        
        halt $zero, $zero, $zero, 0 # halt execution

.word 256 0x3264
.word 257 0xC864
.word 258 0xC8FA