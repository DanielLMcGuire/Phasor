// core/logical/darwin_bsd_aarch64.s
// Logical ops for ARM64 (macOS/BSD) System V ABI

    .text
    .global asm_not
    .global asm_and
    .global asm_or
    .global asm_xor
    .global asm_equal
    .global asm_not_equal
    .global asm_less_than
    .global asm_greater_than
    .global asm_less_equal
    .global asm_greater_equal

asm_not:
    cmp x0, #0
    cset x0, eq
    ret

asm_and:
    cmp x0, #0
    ccmp x1, #0, #0, ne
    cset x0, ne
    ret

asm_or:
    cmp x0, #0
    ccmp x1, #0, #4, eq
    cset x0, ne
    ret

asm_xor:
    cmp x0, #0
    cset x2, ne       // store a != 0 in x2
    cmp x1, #0
    cset x1, ne
    eor x0, x2, x1
    ret

asm_equal:
    cmp x0, x1
    cset x0, eq
    ret

asm_not_equal:
    cmp x0, x1
    cset x0, ne
    ret

asm_less_than:
    cmp x0, x1
    cset x0, lt
    ret

asm_greater_than:
    cmp x0, x1
    cset x0, gt
    ret

asm_less_equal:
    cmp x0, x1
    cset x0, le
    ret

asm_greater_equal:
    cmp x0, x1
    cset x0, ge
    ret
