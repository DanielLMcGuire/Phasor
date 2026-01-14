# core/logical/linux_aarch64.s
# Defines logical operations used by the VM for AArch64 on Linux

    .text
    .global asm_inot
    .global asm_iand
    .global asm_ior
    .global asm_iequal
    .global asm_inot_equal
    .global asm_iless_than
    .global asm_igreater_than
    .global asm_iless_equal
    .global asm_igreater_equal
    .global asm_flnot
    .global asm_fland
    .global asm_flor
    .global asm_flxor
    .global asm_flequal
    .global asm_flnot_equal
    .global asm_flless_than
    .global asm_flgreater_than
    .global asm_flless_equal
    .global asm_flgreater_equal

// int64_t asm_inot(int64_t a)
asm_inot:
    cmp x0, #0
    cset x0, eq
    ret

// int64_t asm_iand(int64_t a, int64_t b)
asm_iand:
    cmp x0, #0
    ccmp x1, #0, #0, ne
    cset x0, ne
    ret

// int64_t asm_ior(int64_t a, int64_t b)
asm_ior:
    cmp x0, #0
    ccmp x1, #0, #4, eq
    cset x0, ne
    ret

// int64_t asm_iequal(int64_t a, int64_t b)
asm_iequal:
    cmp x0, x1
    cset x0, eq
    ret

// int64_t asm_inot_equal(int64_t a, int64_t b)
asm_inot_equal:
    cmp x0, x1
    cset x0, ne
    ret

// int64_t asm_iless_than(int64_t a, int64_t b)
asm_iless_than:
    cmp x0, x1
    cset x0, lt
    ret

// int64_t asm_igreater_than(int64_t a, int64_t b)
asm_igreater_than:
    cmp x0, x1
    cset x0, gt
    ret

// int64_t asm_iless_equal(int64_t a, int64_t b)
asm_iless_equal:
    cmp x0, x1
    cset x0, le
    ret

// int64_t asm_igreater_equal(int64_t a, int64_t b)
asm_igreater_equal:
    cmp x0, x1
    cset x0, ge
    ret

// int64_t asm_flnot(double a)
asm_flnot:
    fcmp d0, #0.0
    cset x0, eq
    ret

// int64_t asm_fland(double a, double b)
asm_fland:
    fcmp d0, #0.0
    cset x2, ne
    fcmp d1, #0.0
    cset x1, ne
    and x0, x2, x1
    ret

// int64_t asm_flor(double a, double b)
asm_flor:
    fcmp d0, #0.0
    cset x2, ne
    fcmp d1, #0.0
    cset x1, ne
    orr x0, x2, x1
    ret

// int64_t asm_flxor(double a, double b)
asm_flxor:
    fcmp d0, #0.0
    cset x2, ne
    fcmp d1, #0.0
    cset x1, ne
    eor x0, x2, x1
    ret

// int64_t asm_flequal(double a, double b)
asm_flequal:
    fcmp d0, d1
    cset x0, eq
    ret

// int64_t asm_flnot_equal(double a, double b)
asm_flnot_equal:
    fcmp d0, d1
    cset x0, ne
    ret

// int64_t asm_flless_than(double a, double b)
asm_flless_than:
    fcmp d0, d1
    cset x0, lt
    ret

// int64_t asm_flgreater_than(double a, double b)
asm_flgreater_than:
    fcmp d0, d1
    cset x0, gt
    ret

// int64_t asm_flless_equal(double a, double b)
asm_flless_equal:
    fcmp d0, d1
    cset x0, le
    ret

// int64_t asm_flgreater_equal(double a, double b)
asm_flgreater_equal:
    fcmp d0, d1
    cset x0, ge
    ret
