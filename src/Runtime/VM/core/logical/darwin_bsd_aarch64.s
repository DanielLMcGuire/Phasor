// core/logical/darwin_bsd_aarch64.s
// Logical ops for ARM64 (macOS/BSD) System V ABI

    .text
    .global asm_inot
    .global asm_iand
    .global asm_ior
    .global asm_ixor
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

asm_inot:
    cmp x0, #0
    cset x0, eq
    ret

asm_iand:
    cmp x0, #0
    ccmp x1, #0, #0, ne
    cset x0, ne
    ret

asm_ior:
    cmp x0, #0
    ccmp x1, #0, #4, eq
    cset x0, ne
    ret

asm_ixor:
    cmp x0, #0
    cset x2, ne       // store a != 0 in x2
    cmp x1, #0
    cset x1, ne
    eor x0, x2, x1
    ret

asm_iequal:
    cmp x0, x1
    cset x0, eq
    ret

asm_inot_equal:
    cmp x0, x1
    cset x0, ne
    ret

asm_iless_than:
    cmp x0, x1
    cset x0, lt
    ret

asm_igreater_than:
    cmp x0, x1
    cset x0, gt
    ret

asm_iless_equal:
    cmp x0, x1
    cset x0, le
    ret

asm_igreater_equal:
    cmp x0, x1
    cset x0, ge
    ret

asm_flnot:
    fcmp d0, #0.0
    cset x0, eq
    ret

asm_fland:
    fcmp d0, #0.0
    cset x2, ne
    fcmp d1, #0.0
    cset x1, ne
    and x0, x2, x1
    ret

asm_flor:
    fcmp d0, #0.0
    cset x2, ne
    fcmp d1, #0.0
    cset x1, ne
    orr x0, x2, x1
    ret

asm_flxor:
    fcmp d0, #0.0
    cset x2, ne
    fcmp d1, #0.0
    cset x1, ne
    eor x0, x2, x1
    ret

asm_flequal:
    fcmp d0, d1
    cset x0, eq
    ret

asm_flnot_equal:
    fcmp d0, d1
    cset x0, ne
    ret

asm_flless_than:
    fcmp d0, d1
    cset x0, lt
    ret

asm_flgreater_than:
    fcmp d0, d1
    cset x0, gt
    ret

asm_flless_equal:
    fcmp d0, d1
    cset x0, le
    ret

asm_flgreater_equal:
    fcmp d0, d1
    cset x0, ge
    ret
