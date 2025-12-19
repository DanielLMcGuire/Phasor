# core/logical/linux_aarch64.s
# Defines logical operations used by the VM for AArch64 on Linux

    .text
    .global asm_not
    .global asm_and
    .global asm_or
    .global asm_equal
    .global asm_not_equal
    .global asm_less_than
    .global asm_greater_than
    .global asm_less_equal
    .global asm_greater_equal

// int64_t asm_not(int64_t a)
asm_not:
    cmp x0, #0
    cset x0, eq
    ret

// int64_t asm_and(int64_t a, int64_t b)
asm_and:
    cmp x0, #0
    ccmp x1, #0, #0, ne
    cset x0, ne
    ret

// int64_t asm_or(int64_t a, int64_t b)
asm_or:
    cmp x0, #0
    ccmp x1, #0, #4, eq
    cset x0, ne
    ret

// int64_t asm_equal(int64_t a, int64_t b)
asm_equal:
    cmp x0, x1
    cset x0, eq
    ret

// int64_t asm_not_equal(int64_t a, int64_t b)
asm_not_equal:
    cmp x0, x1
    cset x0, ne
    ret

// int64_t asm_less_than(int64_t a, int64_t b)
asm_less_than:
    cmp x0, x1
    cset x0, lt
    ret

// int64_t asm_greater_than(int64_t a, int64_t b)
asm_greater_than:
    cmp x0, x1
    cset x0, gt
    ret

// int64_t asm_less_equal(int64_t a, int64_t b)
asm_less_equal:
    cmp x0, x1
    cset x0, le
    ret

// int64_t asm_greater_equal(int64_t a, int64_t b)
asm_greater_equal:
    cmp x0, x1
    cset x0, ge
    ret
