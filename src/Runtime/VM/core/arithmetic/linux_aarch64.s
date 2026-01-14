// core/arithmetic/linux_aarch64.s
// Defines arithmetic operations for VM on Linux AArch64

    .text
    .global asm_iadd
    .global asm_isub
    .global asm_imul
    .global asm_ineg
    .global asm_idiv
    .global asm_imod
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

// int64_t asm_iadd(int64_t a, int64_t b)
asm_iadd:
    add x0, x0, x1
    ret

// int64_t asm_isub(int64_t a, int64_t b)
asm_isub:
    sub x0, x0, x1
    ret

// int64_t asm_imul(int64_t a, int64_t b)
asm_imul:
    mul x0, x0, x1
    ret

// int64_t asm_ineg(int64_t a)
asm_ineg:
    neg x0, x0
    ret

// int64_t asm_idiv(int64_t a, int64_t b)
asm_idiv:
    sdiv x0, x0, x1
    ret

// int64_t asm_imod(int64_t a, int64_t b)
asm_imod:
    sdiv x2, x0, x1      // x2 = a / b
    msub x0, x2, x1, x0  // x0 = a - (x2 * b)
    ret

// double asm_fladd(double a, double b)
asm_fladd:
    fadd d0, d0, d1
    ret

// double asm_flsub(double a, double b)
asm_flsub:
    fsub d0, d0, d1
    ret

// double asm_flmul(double a, double b)
asm_flmul:
    fmul d0, d0, d1
    ret

// double asm_flneg(double a)
asm_flneg:
    fneg d0, d0
    ret

// double asm_fldiv(double a, double b)
asm_fldiv:
    fdiv d0, d0, d1
    ret

// double asm_flmod(double a, double b)
asm_flmod:
    fdiv d2, d0, d1          // d2 = a / b
    frinta d2, d2             // truncate toward zero
    fmul d2, d2, d1          // d2 = trunc(a/b) * b
    fsub d0, d0, d2          // d0 = a - d2
    ret

// double asm_sqrt(double a)
asm_sqrt:
    fsqrt d0, d0
    ret

// double asm_pow(double a, double b)
asm_pow:
    bl pow
    ret

// double asm_log(double a)
asm_log:
    bl log
    ret

// double asm_exp(double a)
asm_exp:
    bl exp
    ret

// double asm_sin(double a)
asm_sin:
    bl sin
    ret

// double asm_cos(double a)
asm_cos:
    bl cos
    ret

// double asm_tan(double a)
asm_tan:
    bl tan
    ret
