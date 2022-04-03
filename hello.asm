  .text
  .globl main
main:
  li t0, 7
  li t1, 0
  sub t0, t1, t0
  li t1, 3
  li t2, 0
  sub t1, t2, t1
  seqz t1, t1
  li t2, 0
  sub t1, t2, t1
  snez t1, t1
  li t2, 4
  li t3, 0
  sub t2, t3, t2
  snez t2, t2
  or t1, t2, t1
  li t2, 2
  mul t1, t2, t1
  li t2, 1
  add t1, t2, t1
  li t2, 5
  slt t1, t2, t1
  li t2, 6
  sub t1, t2, t1
  snez t1, t1
  li t2, 0
  sub t1, t2, t1
  snez t1, t1
  li t2, 0
  sub t1, t2, t1
  snez t1, t1
  and t0, t1, t0
  add a0, a0, t0
  ret
