// arithmetic/darwin_bsd_aarch64.s
// Arithmetic ops for ARM64 (macOS/BSD)

    .text
    .global asm_add
    .global asm_sub
    .global asm_mul
    .global asm_neg
    .global asm_div
    .global asm_mod
    .global asm_sqrt
    .global asm_pow
    .global asm_log
    .global asm_exp
    .global asm_sin
    .global asm_cos
    .global asm_tan

    .extern pow
    .extern log
    .extern exp
    .extern sin
    .extern cos
    .extern tan

asm_add:
    add x0, x0, x1
    ret

asm_sub:
    sub x0, x0, x1
    ret

asm_mul:
    mul x0, x0, x1
    ret

asm_neg:
    neg x0, x0
    ret

asm_div:
    sdiv x0, x0, x1
    ret

asm_mod:
    sdiv x2, x0, x1
    msub x0, x2, x1, x0
    ret

asm_sqrt:
    fsqrt d0, d0
    ret

asm_pow:
    bl pow
    ret

asm_log:
    bl log
    ret

asm_exp:
    bl exp
    ret

asm_sin:
    bl sin
    ret

asm_cos:
    bl cos
    ret

asm_tan:
    bl tan
    ret
