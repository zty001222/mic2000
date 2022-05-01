  .data
  .globl var
var:
  .zero 4
  .zero 4

  .text
  .globl t
t:
  addi sp, sp, -1024
  sw ra, 0(sp)
  la t1, var
  lw t0, 0(t1)
  sw t0, 4(sp)
  lw t0, 4(sp)
  li t1, 1
  add t0, t1, t0
  sw t0, 8(sp)
  lw t0, 8(sp)
  la t1, var
  sw t0, 0(t1)
  li t0, 1
  add a0, zero, t0
  lw ra, 0(sp)
  addi sp, sp, 1024
  ret

  .text
  .globl f
f:
  addi sp, sp, -1024
  sw ra, 0(sp)
  la t1, var
  lw t0, 4(t1)
  sw t0, 4(sp)
  lw t0, 4(sp)
  li t1, 1
  add t0, t1, t0
  sw t0, 8(sp)
  lw t0, 8(sp)
  la t1, var
  sw t0, 4(t1)
  li t0, 0
  add a0, zero, t0
  lw ra, 0(sp)
  addi sp, sp, 1024
  ret

  .text
  .globl main
main:
  addi sp, sp, -1024
  sw ra, 0(sp)
  li t0, 0
  sw t0, 4(sp)
  lw t0, 4(sp)
  sw t0, 8(sp)
  call f
  add t0, zero, a0
  sw t0, 12(sp)
  call f
  add t0, zero, a0
  sw t0, 16(sp)
  lw t0, 8(sp)
  li t1, 0
  add t0, t1, t0
  sw t0, 20(sp)
  lw t0, 20(sp)
  sw t0, 4(sp)
  la t1, var
  lw t0, 0(t1)
  sw t0, 24(sp)
  lw t0, 24(sp)
  add a0, zero, t0
  call putint
  add t0, zero, a0
  sw t0, 28(sp)
  li t0, 32
  add a0, zero, t0
  call putch
  add t0, zero, a0
  sw t0, 32(sp)
  la t1, var
  lw t0, 4(t1)
  sw t0, 36(sp)
  lw t0, 36(sp)
  add a0, zero, t0
  call putint
  add t0, zero, a0
  sw t0, 40(sp)
  li t0, 10
  add a0, zero, t0
  call putch
  add t0, zero, a0
  sw t0, 44(sp)
  lw t0, 4(sp)
  sw t0, 48(sp)
  lw t0, 48(sp)
  add a0, zero, t0
  lw ra, 0(sp)
  addi sp, sp, 1024
  ret
