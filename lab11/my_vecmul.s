.section .text
.globl _start

_start:
    lui     sp, 0x7ff00

    lui     x10, 0x90000
    addi    x10, x10, 0

    lui     x11, 0x90001
    addi    x11, x11, 0

    lui     x12, 0x90002
    addi    x12, x12, 0

    li      t0, 8
    vsetvli t0, t0, e32, ta, ma

    vle32.v v0, (x10)
    vle32.v v1, (x11)

    vmul.vv v2, v0, v1

    vse32.v v2, (x12)

    li      t1, 1
    la      t0, tohost
    sw      t1, 0(t0)
1:
    j       1b

.section .tohost
.align 3
tohost:
    .dword 0
fromhost:
    .dword 0
