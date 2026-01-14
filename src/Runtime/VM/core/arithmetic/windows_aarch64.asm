; arithmetic/windows_aarch64.asm
; Defines arithmetic operations used by the VM for AArch64 on Windows

    AREA    |.text|, CODE, READONLY, ALIGN=2

    EXPORT  asm_iadd
    EXPORT  asm_isub
    EXPORT  asm_imul
    EXPORT  asm_ineg
    EXPORT  asm_idiv
    EXPORT  asm_imod
    EXPORT  asm_fladd
    EXPORT  asm_flsub
    EXPORT  asm_flmul
    EXPORT  asm_flneg
    EXPORT  asm_fldiv
    EXPORT  asm_flmod
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

; int64_t asm_iadd(int64_t a, int64_t b)
; asm_iadd x0 a, x1 b
asm_iadd
    add     x0, x0, x1       ; add a + b
    ret

; int64_t asm_isub(int64_t a, int64_t b)
; asm_isub x0 a, x1 b
asm_isub
    sub     x0, x0, x1       ; subtract b from a
    ret

; int64_t asm_imul(int64_t a, int64_t b)
; asm_imul x0 a, x1 b
asm_imul
    mul     x0, x0, x1       ; multiply a * b
    ret

; int64_t asm_ineg(int64_t a)
; asm_ineg x0 a
asm_ineg
    neg     x0, x0           ; negate a
    ret

; int64_t asm_idiv(int64_t a, int64_t b)
; asm_idiv x0 a, x1 b
asm_idiv
    sdiv    x0, x0, x1       ; signed division a / b
    ret

; int64_t asm_imod(int64_t a, int64_t b)
; asm_imod x0 a, x1 b
asm_imod
    sdiv    x2, x0, x1       ; x2 = a / b
    msub    x0, x2, x1, x0   ; x0 = a - (x2 * b) -> remainder
    ret

; double asm_fladd(double a, double b)
; asm_fladd d0 a, d1 b
asm_fladd
    fadd    d0, d0, d1       ; a + b
    ret

; double asm_flsub(double a, double b)
; asm_flsub d0 a, d1 b
asm_flsub
    fsub    d0, d0, d1       ; a - b
    ret

; double asm_flmul(double a, double b)
; asm_flmul d0 a, d1 b
asm_flmul
    fmul    d0, d0, d1       ; a * b
    ret

; double asm_flneg(double a)
; asm_flneg d0 a
asm_flneg
    fneg    d0, d0           ; -a
    ret

; double asm_fldiv(double a, double b)
; asm_fldiv d0 a, d1 b
asm_fldiv
    fdiv    d0, d0, d1       ; a / b
    ret

; double asm_flmod(double a, double b)
; asm_flmod d0 a, d1 b
asm_flmod
    fdiv    d2, d0, d1       ; d2 = a / b
    frinta  d2, d2           ; truncate toward zero
    fmul    d2, d2, d1       ; d2 = trunc(a/b) * b
    fsub    d0, d0, d2       ; d0 = a - d2 -> remainder
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
