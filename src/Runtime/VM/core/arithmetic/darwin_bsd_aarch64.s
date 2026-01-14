// arithmetic/darwin_bsd_aarch64.s
// Arithmetic ops for ARM64 (macOS/BSD)

    .text
    .global asm_iadd
    .global asm_isub
    .global asm_imul
    .global asm_ineg
    .global asm_idiv
    .global asm_imod
    .global asm_fladd
    .global asm_flsub
    .global asm_flmul
    .global asm_flneg
    .global asm_fldiv
    .global asm_flmod
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

asm_iadd:
    add x0, x0, x1
    ret

asm_isub:
    sub x0, x0, x1
    ret

asm_imul:
    mul x0, x0, x1
    ret

asm_ineg:
    neg x0, x0
    ret

asm_idiv:
    sdiv x0, x0, x1
    ret

asm_imod:
    sdiv x2, x0, x1
    msub x0, x2, x1, x0
    ret

asm_fladd:
    fadd d0, d0, d1
    ret

asm_flsub:
    fsub d0, d0, d1
    ret

asm_flmul:
    fmul d0, d0, d1
    ret

asm_flneg:
    fneg d0, d0
    ret

asm_fldiv:
    fdiv d0, d0, d1
    ret

asm_flmod:
    // compute remainder: d0 - d1 * trunc(d0/d1)
    frinta d2, d0, #0   // truncate towards zero (integer part)
    fdiv d2, d0, d1
    frinta d2, d2, #0
    fmul d2, d2, d1
    fsub d0, d0, d2
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
