; core/logical/windows_aarch64.asm
; Defines native safe functions used by the VM for AArch64 on Windows

    AREA    |.text|, CODE, READONLY, ALIGN=2

    
    EXPORT  asm_inot
    EXPORT  asm_iand
    EXPORT  asm_ior
    EXPORT  asm_iequal
    EXPORT  asm_inot_equal
    EXPORT  asm_iless_than
    EXPORT  asm_igreater_than
    EXPORT  asm_iless_equal
    EXPORT  asm_igreater_equal
    EXPORT  asm_flnot
    EXPORT  asm_fland
    EXPORT  asm_flor
    EXPORT  asm_flequal
    EXPORT  asm_flnot_equal
    EXPORT  asm_flless_than
    EXPORT  asm_flgreater_than
    EXPORT  asm_flless_equal
    EXPORT  asm_flgreater_equal

; int64_t asm_inot(int64_t a)
; asm_inot x0 a
asm_inot
    cmp     x0, #0           ; compare a to 0
    cset    x0, eq           ; x0 = 1 if a == 0, else 0
    ret

; int64_t asm_iand(int64_t a, int64_t b)
; asm_iand x0 a, x1 b
asm_iand
    cmp     x0, #0
    ccmp    x1, #0, #0, ne   ; if a != 0, compare b; else set flags to 0
    cset    x0, ne           ; x0 = 1 if both non-zero
    ret

; int64_t asm_ior(int64_t a, int64_t b)
; asm_ior x0 a, x1 b
asm_ior
    cmp     x0, #0
    ccmp    x1, #0, #4, eq   ; if a == 0, compare b; else set flags to non-zero
    cset    x0, ne           ; x0 = 1 if either non-zero
    ret

; int64_t asm_iequal(int64_t a, int64_t b)
; asm_iequal x0 a, x1 b
asm_iequal
    cmp     x0, x1
    cset    x0, eq           ; x0 = 1 if a == b
    ret

; int64_t asm_inot_equal(int64_t a, int64_t b)
; asm_inot_equal x0 a, x1 b
asm_inot_equal
    cmp     x0, x1
    cset    x0, ne           ; x0 = 1 if a != b
    ret

; int64_t asm_iless_than(int64_t a, int64_t b)
; asm_iless_than x0 a, x1 b
asm_iless_than
    cmp     x0, x1
    cset    x0, lt           ; x0 = 1 if a < b
    ret

; int64_t asm_igreater_than(int64_t a, int64_t b)
; asm_igreater_than x0 a, x1 b
asm_igreater_than
    cmp     x0, x1
    cset    x0, gt           ; x0 = 1 if a > b
    ret

; int64_t asm_iless_equal(int64_t a, int64_t b)
; asm_iless_equal x0 a, x1 b
asm_iless_equal
    cmp     x0, x1
    cset    x0, le           ; x0 = 1 if a <= b
    ret

; int64_t asm_igreater_equal(int64_t a, int64_t b)
; asm_igreater_equal x0 a, x1 b
asm_igreater_equal
    cmp     x0, x1
    cset    x0, ge           ; x0 = 1 if a >= b
    ret

    END

; int64_t asm_flnot(double a)
; asm_flnot x0 a
asm_flnot
    fcmp    d0, #0.0
    cset    x0, eq           ; x0 = 1 if d0 == 0.0
    ret

; int64_t asm_fland(double a, double b)
; asm_fland x0 a, x1 b
asm_fland
    fcmp    d0, #0.0
    cset    x2, ne           ; x2 = d0 != 0
    fcmp    d1, #0.0
    cset    x1, ne           ; x1 = d1 != 0
    and     x0, x2, x1       ; x0 = d0 && d1
    ret

; int64_t asm_flor(double a, double b)
; asm_flor x0 a, x1 b
asm_flor
    fcmp    d0, #0.0
    cset    x2, ne
    fcmp    d1, #0.0
    cset    x1, ne
    orr     x0, x2, x1       ; x0 = d0 || d1
    ret

; int64_t asm_flxor(double a, double b)
; asm_flxor x0 a, x1 b
asm_flxor
    fcmp    d0, #0.0
    cset    x2, ne
    fcmp    d1, #0.0
    cset    x1, ne
    eor     x0, x2, x1       ; x0 = d0 != d1
    ret

; int64_t asm_flequal(double a, double b)
; asm_flequal x0 a, x1 b
asm_flequal
    fcmp    d0, d1
    cset    x0, eq           ; x0 = 1 if d0 == d1
    ret

; int64_t asm_flnot_equal(double a, double b)
; asm_flnot_equal x0 a, x1 b
asm_flnot_equal
    fcmp    d0, d1
    cset    x0, ne           ; x0 = 1 if d0 != d1
    ret

; int64_t asm_flless_than(double a, double b)
; asm_flless_than x0 a, x1 b
asm_flless_than
    fcmp    d0, d1
    cset    x0, lt           ; x0 = 1 if d0 < d1
    ret

; int64_t asm_flgreater_than(double a, double b)
; asm_flgreater_than x0 a, x1 b
asm_flgreater_than
    fcmp    d0, d1
    cset    x0, gt           ; x0 = 1 if d0 > d1
    ret

; int64_t asm_flless_equal(double a, double b)
; asm_flless_equal x0 a, x1 b
asm_flless_equal
    fcmp    d0, d1
    cset    x0, le           ; x0 = 1 if d0 <= d1
    ret

; int64_t asm_flgreater_equal(double a, double b)
; asm_flgreater_equal x0 a, x1 b
asm_flgreater_equal
    fcmp    d0, d1
    cset    x0, ge           ; x0 = 1 if d0 >= d1
    ret
