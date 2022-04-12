  .text
  .globl main
entry:
  li t0, 1
  sw t0, 0(sp)
  lw t0, 0(sp)
  sw t0, 4(sp)
  lw t0, 4(sp)
  bnez t0, then0
  j end0
then0:
  li t1, 2
  sw t1, 0(sp)
  j end0
end0:
  lw t1, 0(sp)
  sw t1, 8(sp)
  lw t1, 8(sp)
  add a0, a0, t1
  addi sp, sp, 1024
  ret
