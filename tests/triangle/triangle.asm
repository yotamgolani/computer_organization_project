# triangle.asm
.word 256 4112
.word 257 8208
.word 258 8224
add $s0 $zero $imm 20
add $s1 $zero $imm 21
add $s2 $zero $imm 22
add $gp $zero $imm 255
lw $t0 $zero $imm 256
lw $t1 $zero $imm 257
lw $t2 $zero $imm 258
sub $t2 $t2 $t1 0
sub $t1 $t1 $t0 0
add $v0 $zero $imm 8
srl $t1 $t1 $v0 0
add $a0 $t0 $zero 0
add $a1 $zero $zero 0
add $a2 $zero $zero 0
add $a3 $zero $zero 0
rowloop:
bgt $imm $a3 $t1 finish
add $v0 $zero $zero 0
colloop:
bgt $imm $v0 $a1 rowdone
add $t0 $a0 $v0 0
out $t0 $s0 $zero 0
out $gp $s1 $zero 0
out $imm $s2 $zero 1
add $v0 $v0 $imm 1
beq $imm $zero $zero colloop
rowdone:
add $a2 $a2 $t2 0
bresenhamloop:
blt $imm $a2 $t1 nextrow
add $a1 $a1 $imm 1
sub $a2 $a2 $t1 0
beq $imm $zero $zero bresenhamloop
nextrow:
add $a0 $a0 $imm 256
add $a3 $a3 $imm 1
beq $imm $zero $zero rowloop
finish:
halt $zero $zero $zero 0