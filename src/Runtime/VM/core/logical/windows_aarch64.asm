; core/logical/windows_aarch64.asm
; Defines native safe functions used by the VM for AArch64 on Windows

    AREA    |.text|, CODE, READONLY, ALIGN=2

    
    EXPORT  asm_not
    EXPORT  asm_and
    EXPORT  asm_or
    EXPORT  asm_equal
    EXPORT  asm_not_equal
    EXPORT  asm_less_than
    EXPORT  asm_greater_than
    EXPORT  asm_less_equal
    EXPORT  asm_greater_equal

; int64_t asm_not(int64_t a)
; asm_not x0 a
asm_not
    cmp     x0, #0           ; compare a to 0
    cset    x0, eq           ; x0 = 1 if a == 0, else 0
    ret

; int64_t asm_and(int64_t a, int64_t b)
; asm_and x0 a, x1 b
asm_and
    cmp     x0, #0
    ccmp    x1, #0, #0, ne   ; if a != 0, compare b; else set flags to 0
    cset    x0, ne           ; x0 = 1 if both non-zero
    ret

; int64_t asm_or(int64_t a, int64_t b)
; asm_or x0 a, x1 b
asm_or
    cmp     x0, #0
    ccmp    x1, #0, #4, eq   ; if a == 0, compare b; else set flags to non-zero
    cset    x0, ne           ; x0 = 1 if either non-zero
    ret

; int64_t asm_equal(int64_t a, int64_t b)
; asm_equal x0 a, x1 b
asm_equal
    cmp     x0, x1
    cset    x0, eq           ; x0 = 1 if a == b
    ret

; int64_t asm_not_equal(int64_t a, int64_t b)
; asm_not_equal x0 a, x1 b
asm_not_equal
    cmp     x0, x1
    cset    x0, ne           ; x0 = 1 if a != b
    ret

; int64_t asm_less_than(int64_t a, int64_t b)
; asm_less_than x0 a, x1 b
asm_less_than
    cmp     x0, x1
    cset    x0, lt           ; x0 = 1 if a < b
    ret

; int64_t asm_greater_than(int64_t a, int64_t b)
; asm_greater_than x0 a, x1 b
asm_greater_than
    cmp     x0, x1
    cset    x0, gt           ; x0 = 1 if a > b
    ret

; int64_t asm_less_equal(int64_t a, int64_t b)
; asm_less_equal x0 a, x1 b
asm_less_equal
    cmp     x0, x1
    cset    x0, le           ; x0 = 1 if a <= b
    ret

; int64_t asm_greater_equal(int64_t a, int64_t b)
; asm_greater_equal x0 a, x1 b
asm_greater_equal
    cmp     x0, x1
    cset    x0, ge           ; x0 = 1 if a >= b
    ret

    END