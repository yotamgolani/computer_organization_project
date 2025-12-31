# comment
.word 256 5
.word 257 2
add $sp $zero $imm 2048
lw $a0 $zero $imm 256
lw $a1 $zero $imm 257
jal $ra $imm $zero binom
sw $v0 $zero $imm 258
halt $zero $zero $zero 0
binom:
add $sp $sp $imm -4
sw $ra $sp $imm 3
sw $s0 $sp $imm 2
sw $s1 $sp $imm 1
sw $s2 $sp $imm 0
beq $imm $a1 $zero ret1
beq $imm $a0 $a1 ret1
add $s0 $a0 $zero 0
add $s1 $a1 $zero 0
add $a0 $s0 $imm -1
add $a1 $s1 $imm -1
jal $ra $imm $zero binom
add $s2 $v0 $zero 0
add $a0 $s0 $imm -1
add $a1 $s1 $zero 0
jal $ra $imm $zero binom
add $v0 $v0 $s2 0
beq $imm $zero $zero end
ret1:
add $v0 $zero $imm 1
end:
lw $ra $sp $imm 3
lw $s0 $sp $imm 2
lw $s1 $sp $imm 1
lw $s2 $sp $imm 0
add $sp $sp $imm 4
beq $ra $zero $zero 0