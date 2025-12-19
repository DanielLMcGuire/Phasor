; arithmetic/windows_x86_64.asm
; Defines arithmetic operations used by the VM for x86_64 on Windows

; PUBLIC SYMBOLS
PUBLIC asm_add
PUBLIC asm_sub
PUBLIC asm_mul
PUBLIC asm_neg
PUBLIC asm_div
PUBLIC asm_mod
PUBLIC asm_sqrt
PUBLIC asm_pow
PUBLIC asm_log
PUBLIC asm_exp
PUBLIC asm_sin
PUBLIC asm_cos
PUBLIC asm_tan

EXTERN sqrt:PROC
EXTERN pow:PROC
EXTERN log:PROC
EXTERN exp:PROC
EXTERN sin:PROC
EXTERN cos:PROC
EXTERN tan:PROC

.CODE

; int64_t asm_add(int64_t a, int64_t b)
; asm_add rcx a, rdx b
asm_add PROC
    mov rax, rcx      ; copy first argument a into return register
    add rax, rdx      ; add second argument b
    ret
asm_add ENDP

; int64_t asm_sub(int64_t a, int64_t b)
; asm_sub rcx a, rdx b
asm_sub PROC
    mov rax, rcx      ; copy a
    sub rax, rdx      ; subtract b
    ret
asm_sub ENDP

; int64_t asm_mul(int64_t a, int64_t b)
; asm_mul rcx a, rdx b
asm_mul PROC
    mov rax, rcx      ; copy a
    imul rax, rdx     ; multiply by b
    ret
asm_mul ENDP

; int64_t asm_neg(int64_t a)
asm_neg PROC
    mov rax, rcx      ; copy a
    neg rax           ; negate value
    ret
asm_neg ENDP

; int64_t asm_div(int64_t a, int64_t b)
; asm_div rcx a, rdx b
asm_div PROC
    mov rax, rcx      ; copy dividend into rax
    mov r8, rdx       ; save divisor in r8 to avoid clobber from cqo
    cqo                ; sign-extend rax into rdx:rax
    idiv r8           ; divide by b
    ret
asm_div ENDP

; int64_t asm_mod(int64_t a, int64_t b)
; asm_mod rcx a, rdx b
asm_mod PROC
    mov rax, rcx      ; copy dividend
    mov r8, rdx       ; save divisor
    cqo
    idiv r8           ; divide, remainder in rdx
    mov rax, rdx      ; return remainder
    ret
asm_mod ENDP

; double asm_sqrt(double a)
; asm_sqrt xmm0 a
asm_sqrt PROC
    sub rsp, 40       ; shadow space (32) + alignment (8)
    call sqrt
    add rsp, 40
    ret
asm_sqrt ENDP

; double asm_pow(double a, double b)
; asm_pow xmm0 a, xmm1 b
asm_pow PROC
    sub rsp, 40
    call pow
    add rsp, 40
    ret
asm_pow ENDP

; double asm_log(double a)
; asm_log xmm0 a
asm_log PROC
    sub rsp, 40
    call log
    add rsp, 40
    ret
asm_log ENDP

; double asm_exp(double a)
; asm_exp xmm0 a
asm_exp PROC
    sub rsp, 40
    call exp
    add rsp, 40
    ret
asm_exp ENDP

; double asm_sin(double a)
; asm_sin xmm0 a
asm_sin PROC
    sub rsp, 40
    call sin
    add rsp, 40
    ret
asm_sin ENDP

; double asm_cos(double a)
; asm_cos xmm0 a
asm_cos PROC
    sub rsp, 40
    call cos
    add rsp, 40
    ret
asm_cos ENDP

; double asm_tan(double a)
; asm_tan xmm0 a
asm_tan PROC
    sub rsp, 40
    call tan
    add rsp, 40
    ret
asm_tan ENDP

END