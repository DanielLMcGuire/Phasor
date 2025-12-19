; arithmetic/windows_aarch64.asm
; Defines arithmetic operations used by the VM for AArch64 on Windows

    AREA    |.text|, CODE, READONLY, ALIGN=2

    EXPORT  asm_add
    EXPORT  asm_sub
    EXPORT  asm_mul
    EXPORT  asm_neg
    EXPORT  asm_div
    EXPORT  asm_mod
    EXPORT  asm_sqrt
    EXPORT  asm_pow
    EXPORT  asm_log
    EXPORT  asm_exp
    EXPORT  asm_sin
    EXPORT  asm_cos
    EXPORT  asm_tan

    IMPORT  pow
    IMPORT  log
    IMPORT  exp
    IMPORT  sin
    IMPORT  cos
    IMPORT  tan

; int64_t asm_add(int64_t a, int64_t b)
; asm_add x0 a, x1 b
asm_add
    add     x0, x0, x1       ; add a + b
    ret

; int64_t asm_sub(int64_t a, int64_t b)
; asm_sub x0 a, x1 b
asm_sub
    sub     x0, x0, x1       ; subtract b from a
    ret

; int64_t asm_mul(int64_t a, int64_t b)
; asm_mul x0 a, x1 b
asm_mul
    mul     x0, x0, x1       ; multiply a * b
    ret

; int64_t asm_neg(int64_t a)
; asm_neg x0 a
asm_neg
    neg     x0, x0           ; negate a
    ret

; int64_t asm_div(int64_t a, int64_t b)
; asm_div x0 a, x1 b
asm_div
    sdiv    x0, x0, x1       ; signed division a / b
    ret

; int64_t asm_mod(int64_t a, int64_t b)
; asm_mod x0 a, x1 b
asm_mod
    sdiv    x2, x0, x1       ; x2 = a / b
    msub    x0, x2, x1, x0   ; x0 = a - (x2 * b) -> remainder
    ret

; double asm_sqrt(double a)
; asm_sqrt d0 a
asm_sqrt
    fsqrt   d0, d0           ; square root (double)
    ret

; double asm_pow(double a, double b)
; asm_pow d0 a, d1 b
asm_pow
    b       pow              ; tail call to CRT pow

; double asm_log(double a)
; asm_log d0 a
asm_log
    b       log              ; tail call to CRT log

; double asm_exp(double a)
; asm_exp d0 a
asm_exp
    b       exp              ; tail call to CRT exp

; double asm_sin(double a)
; asm_sin d0 a
asm_sin
    b       sin              ; tail call to CRT sin

; double asm_cos(double a)
; asm_cos d0 a
asm_cos
    b       cos              ; tail call to CRT cos

; double asm_tan(double a)
; asm_tan d0 a
asm_tan
    b       tan              ; tail call to CRT tan

    END
