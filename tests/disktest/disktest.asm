        # This program sums 8 sectors (0-7) from disk and writes the result to sector 8.
        # Each sector is 128 words.
        
        add $s0, $zero, $imm, 0x300 # s0 = accum_buffer start address
        add $s2, $zero, $imm, 128   # s2 = constant 128 (sector size)
        
        # Initialize accum_buffer with 0
        add $t0, $zero, $zero, 0    # t0 = i = 0
init_loop:
        sw $zero, $s0, $t0, 0       # accum_buffer[i] = 0
        add $t0, $t0, $imm, 1       # i++
        blt $imm, $t0, $s2, init_loop # if i < 128, repeat

        # Loop through sectors 0-7
        add $s1, $zero, $zero, 0    # s1 = current sector number (0 to 7)
sector_loop:
        # Read sector s1 into 0x200
        out $s1, $imm, $zero, 15    # IORegister[15] (disksector) = s1
        add $t0, $zero, $imm, 0x200
        out $t0, $imm, $zero, 16    # IORegister[16] (diskbuffer) = 0x200
        add $t0, $zero, $imm, 1
        out $t0, $imm, $zero, 14    # IORegister[14] (diskcmd) = 1 (read)
        
        # Wait for disk to be ready (diskstatus == 0)
wait_read:
        in $t0, $imm, $zero, 17     # t0 = IORegister[17] (diskstatus)
        bne $imm, $t0, $zero, wait_read
        
        # Accumulate curr_sector_buffer (0x200) into accum_buffer (0x300)
        add $t0, $zero, $zero, 0    # t0 = i = 0
accum_loop:
        add $t2, $t0, $imm, 0x200   # t2 = 0x200 + i
        lw $t1, $t2, $imm, 0        # t1 = curr_sector_buffer[i]
        add $t2, $t0, $imm, 0x300   # t2 = 0x300 + i
        lw $t3, $t2, $imm, 0        # t3 = accum_buffer[i]
        add $t3, $t3, $t1, 0        # t3 = t3 + t1
        sw $t3, $t2, $imm, 0        # accum_buffer[i] = t3
        
        add $t0, $t0, $imm, 1       # i++
        blt $imm, $t0, $s2, accum_loop # if i < 128, repeat
        
        add $s1, $s1, $imm, 1       # next sector
        add $t4, $zero, $imm, 8
        blt $imm, $s1, $t4, sector_loop # if s1 < 8, repeat
        
        # Write accum_buffer (0x300) to sector 8
        add $t0, $zero, $imm, 8
        out $t0, $imm, $zero, 15    # IORegister[15] (disksector) = 8
        add $t0, $zero, $imm, 0x300
        out $t0, $imm, $zero, 16    # IORegister[16] (diskbuffer) = 0x300
        add $t0, $zero, $imm, 2
        out $t0, $imm, $zero, 14    # IORegister[14] (diskcmd) = 2 (write)

        # Wait for disk to be ready
wait_write:
        in $t0, $imm, $zero, 17     # t0 = IORegister[17] (diskstatus)
        bne $imm, $t0, $zero, wait_write
        
        halt $zero, $zero, $zero, 0 # halt execution
