.section .text.init
.globl _start
_start:
# Please do not modify the code above
# TODO: write your code here
la t0, array
li t1, 4
li t2, 0

loop:
ld t3, 0(t0)
add t2, t2, t3
addi t0, t0, 8
addi t1, t1, -1
bnez t1, loop

la t4, result
sd t2, 0(t4)
# Return the result. Please do not modify the code below.
lw a0, result
sll a0, a0, 1
or a0, a0, 1
write_tohost:
sw a0, tohost, t5
sw zero, tohost + 4, t5
j write_tohost

.section .data
.align 3
array:
.dword 1
.dword 2
.dword 3
.dword 4
result:
.dword 0

# tohost definition
.pushsection .tohost,"aw",@progbits
.align 6
.global tohost
tohost:
.dword 0
.size tohost, 8
.align 6
.global fromhost
fromhost:
.dword 0
.size fromhost, 8
.popsection
