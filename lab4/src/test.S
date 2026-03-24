.section .text.init
.globl _start

_start:
# Please do not modify the code above
# TODO: Construct three types of hazards below:
#       1. Data hazard (observe bypassing)
#       2. Load-use data hazard (observe stall)
#       3. Control hazard (observe prediction)

    la s0, array
    la s1, result

    # 1. Data hazard:
    # addi writes t0, the next add uses t0 immediately.
    addi t0, zero, 5
    add t1, t0, t0
    sub t2, t1, t0

    # 2. Load-use hazard:
    # ld writes t3, the next add uses t3 immediately.
    ld t3, 0(s0)
    add t4, t3, t0

    # 3. Control hazard:
    # First branch is taken, second branch is not taken.
    addi a1, zero, 1
    addi a2, zero, 1
    beq a1, a2, taken_branch
    addi a3, zero, 99

taken_branch:
    addi a4, zero, 7
    addi a5, zero, 8
    beq a4, a5, not_taken_target
    addi a6, zero, 1
    j branch_done

not_taken_target:
    addi a6, zero, 2

branch_done:
    sw zero, 0(s1)

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
