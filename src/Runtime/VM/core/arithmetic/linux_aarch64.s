// core/arithmetic/linux_aarch64.s
// Defines arithmetic operations for VM on Linux AArch64

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

// int64_t asm_add(int64_t a, int64_t b)
asm_add:
    add x0, x0, x1
    ret

// int64_t asm_sub(int64_t a, int64_t b)
asm_sub:
    sub x0, x0, x1
    ret

// int64_t asm_mul(int64_t a, int64_t b)
asm_mul:
    mul x0, x0, x1
    ret

// int64_t asm_neg(int64_t a)
asm_neg:
    neg x0, x0
    ret

// int64_t asm_div(int64_t a, int64_t b)
asm_div:
    sdiv x0, x0, x1
    ret

// int64_t asm_mod(int64_t a, int64_t b)
asm_mod:
    sdiv x2, x0, x1      // x2 = a / b
    msub x0, x2, x1, x0  // x0 = a - (x2 * b)
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
