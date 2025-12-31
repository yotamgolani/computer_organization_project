# disktest.asm
add $s0 $zero $imm 14
add $s1 $zero $imm 15
add $s2 $zero $imm 16
add $gp $zero $imm 17
add $a2 $zero $imm 2048
add $a3 $zero $imm 2304
add $t0 $zero $zero 0
add $t1 $zero $imm 128
clearloop:
beq $imm $t0 $t1 starttest
add $t2 $a2 $t0 0
sw $zero $t2 $zero 0
add $t0 $t0 $imm 1
beq $imm $zero $zero clearloop
starttest:
add $a0 $zero $zero 0
add $a1 $zero $imm 8
sectorloop:
beq $imm $a0 $a1 writesector
poll1:
in $t0 $gp $zero 0
bne $imm $t0 $zero poll1
out $a3 $s2 $zero 0
out $a0 $s1 $zero 0
out $imm $s0 $zero 1
poll2:
in $t0 $gp $zero 0
bne $imm $t0 $zero poll2
add $t0 $zero $zero 0
add $t1 $zero $imm 128
sumloop:
beq $imm $t0 $t1 nextsector
add $t2 $a3 $t0 0
lw $v0 $t2 $zero 0
add $sp $a2 $t0 0
lw $ra $sp $zero 0
add $ra $ra $v0 0
sw $ra $sp $zero 0
add $t0 $t0 $imm 1
beq $imm $zero $zero sumloop
nextsector:
add $a0 $a0 $imm 1
beq $imm $zero $zero sectorloop
writesector:
poll3:
in $t0 $gp $zero 0
bne $imm $t0 $zero poll3
out $a2 $s2 $zero 0
add $t0 $zero $imm 8
out $t0 $s1 $zero 0
add $t0 $zero $imm 2
out $t0 $s0 $zero 0
poll4:
in $t0 $gp $zero 0
bne $imm $t0 $zero poll4
halt $zero $zero $zero 0