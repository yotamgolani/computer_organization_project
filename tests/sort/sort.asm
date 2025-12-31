# sort.asm
.word 256 12
.word 257 5
.word 258 8
.word 259 1
.word 260 9
.word 261 14
.word 262 3
.word 263 10
.word 264 0
.word 265 15
.word 266 2
.word 267 7
.word 268 6
.word 269 13
.word 270 4
.word 271 11
add $s0 $zero $imm 256
add $s1 $zero $imm 15
outerloop:
beq $imm $s1 $zero finish
add $t0 $zero $zero 0
innerloop:
beq $imm $t0 $s1 nextouter
add $t1 $s0 $t0 0
lw $t2 $t1 $zero 0
lw $a0 $t1 $imm 1
bgt $imm $t2 $a0 swaplabel
beq $imm $zero $zero loopinc
swaplabel:
sw $a0 $t1 $zero 0
sw $t2 $t1 $imm 1
loopinc:
add $t0 $t0 $imm 1
beq $imm $zero $zero innerloop
nextouter:
add $s1 $s1 $imm -1
beq $imm $zero $zero outerloop
finish:
halt $zero $zero $zero 0
