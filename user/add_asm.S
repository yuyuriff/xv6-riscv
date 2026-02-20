  #define MAX_SIZE 32
  #define N_SYMB 10
  #define R_SYMB 13

  .data
    buf: .space MAX_SIZE
    a: .word 0
    b: .word 0

    flush_buf: .byte 0

    test_fmt: .asciz "|%s|\n"
    sum_fmt: .asciz "%lld\n"
    empty_fmt: .asciz "Empty input\n"
    overflow_fmt: .asciz "Buffer overflow\n"

  .text
  .global main

  main:
    la a0, buf
    li a1, MAX_SIZE
    call gets

    la t1, buf
    li t5, N_SYMB
    li t6, R_SYMB
    j find_newline

  gets_check_passed:

    la a0, test_fmt
    la a1, buf
    call printf

    la a0, buf
    la a1, a
    la a2, b
    call parse_ints

    lw t0, a
    lw t1, b
    add t2, t0, t1

    la a0, sum_fmt
    mv a1, t2
    call printf

    li a7, 2
    li a0, 0
    ecall

  find_newline:
    lbu t2, 0(t1)
    beq t2, zero, reached_end

    beq t2, t5, found_newline
    beq t2, t6, found_newline

    addi t1, t1, 1
    j find_newline

  reached_end:
    la t4, buf
    beq t1, t4, empty_input

  flush_input:
    li a7, 5
    li a0, 0
    la a1, flush_buf
    li a2, 1
    ecall
    ble a0, zero, flush_stop

    lbu t3, flush_buf
    beq t3, t5, flush_stop
    beq t3, t6, flush_stop

    j flush_input

  flush_stop:
    la a0, overflow_fmt
    call printf
    j exit_1

  empty_input:
    li a0, 2
    la a1, empty_fmt
    call fprintf

    j exit_1

  found_newline:
    sb zero, 0(t1)
    j gets_check_passed

  exit_1:
    li a7, 2
    li a0, 1
    ecall
