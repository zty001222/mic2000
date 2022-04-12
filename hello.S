  .text
  .globl main
main:
  addi sp, sp, -128
  li t0, 10
  li t1, 11
  add t0, t1, t0
  sw t0, 4(sp)
  lw t0, 4(sp)
  sw t0, 8(sp)
  lw t0, 8(sp)
  sw t0, 12(sp)
  lw t0, 12(sp)
  li t1, 1
  add t0, t1, t0
  sw t0, 16(sp)
  lw t0, 16(sp)
  sw t0, 0(sp)
  lw t0, 4(sp)
  li t1, 7
  mul t0, t1, t0
  sw t0, 20(sp)
  lw t0, 0(sp)
  sw t0, 24(sp)
  lw t0, 20(sp)
  lw t1, 24(sp)
  sub t0, t0, t1
  sw t0, 28(sp)
  lw t0, 28(sp)
  sw t0, 32(sp)
  lw t0, 8(sp)
  sw t0, 36(sp)
  lw t0, 20(sp)
  lw t1, 36(sp)
  sub t0, t0, t1
  sw t0, 40(sp)
  lw t0, 40(sp)
  sw t0, 44(sp)
  lw t0, 44(sp)
  sw t0, 48(sp)
  lw t0, 48(sp)
  li t1, 1
  mul t0, t1, t0
  sw t0, 52(sp)
  lw t0, 52(sp)
  li t1, 1
  mul t0, t1, t0
  sw t0, 56(sp)
  lw t0, 56(sp)
  li t1, 1
  mul t0, t1, t0
  sw t0, 60(sp)
  lw t0, 60(sp)
  sw t0, 44(sp)
  lw t0, 44(sp)
  sw t0, 64(sp)
  lw t0, 32(sp)
  sw t0, 68(sp)
  lw t0, 68(sp)
  lw t1, 64(sp)
  add t0, t1, t0
  sw t0, 72(sp)
  lw t0, 72(sp)
  add a0, a0, t0
  addi sp, sp, 128
  ret
