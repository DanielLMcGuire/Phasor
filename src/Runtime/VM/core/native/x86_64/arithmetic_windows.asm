; arithmetic/windows_x86_64.asm
; Defines arithmetic operations used by the VM for x86_64 on Windows

; PUBLIC SYMBOLS
PUBLIC asm_iadd
PUBLIC asm_isub
PUBLIC asm_imul
PUBLIC asm_ineg
PUBLIC asm_idiv
PUBLIC asm_imod
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

; int64_t asm_iadd(int64_t a, int64_t b)
; asm_iadd rcx a, rdx b
asm_iadd PROC
    mov rax, rcx      ; copy first argument a into return register
    add rax, rdx      ; add second argument b
    ret
asm_iadd ENDP

; int64_t asm_isub(int64_t a, int64_t b)
; asm_isub rcx a, rdx b
asm_isub PROC
    mov rax, rcx      ; copy a
    sub rax, rdx      ; subtract b
    ret
asm_isub ENDP

; int64_t asm_imul(int64_t a, int64_t b)
; asm_imul rcx a, rdx b
asm_imul PROC
    mov rax, rcx      ; copy a
    imul rax, rdx     ; multiply by b
    ret
asm_imul ENDP

; int64_t asm_ineg(int64_t a)
; asm_ineg rcx a
asm_ineg PROC
    mov rax, rcx      ; copy a
    neg rax           ; negate value
    ret
asm_ineg ENDP

; int64_t asm_idiv(int64_t a, int64_t b)
; asm_idiv rcx a, rdx b
asm_idiv PROC
    mov rax, rcx      ; copy dividend into rax
    mov r8, rdx       ; save divisor in r8 to avoid clobber from cqo
    cqo                ; sign-extend rax into rdx:rax
    idiv r8           ; divide by b
    ret
asm_idiv ENDP

; int64_t asm_imod(int64_t a, int64_t b)
; asm_imod rcx a, rdx b
asm_imod PROC
    mov rax, rcx      ; copy dividend
    mov r8, rdx       ; save divisor
    cqo
    idiv r8           ; divide, remainder in rdx
    mov rax, rdx      ; return remainder
    ret
asm_imod ENDP

; double asm_fladd(double a, double b)
; asm_fladd xmm0 a, xmm1 b
asm_fladd PROC
    addsd xmm0, xmm1       ; a + b
    ret
asm_fladd ENDP

; double asm_flsub(double a, double b)
; asm_flsub xmm0 a, xmm1 b
asm_flsub PROC
    subsd xmm0, xmm1       ; a - b
    ret
asm_flsub ENDP

; double asm_flmul(double a, double b)
; asm_flmul xmm0 a, xmm1 b
asm_flmul PROC
    mulsd xmm0, xmm1       ; a * b
    ret
asm_flmul ENDP

; double asm_flneg(double a)
; asm_flneg xmm0 a
asm_flneg PROC
    xorpd xmm1, xmm1       ; zero in xmm1
    subsd xmm0, xmm1       ; 0 - a
    ret
asm_flneg ENDP

; double asm_fldiv(double a, double b)
; asm_fldiv xmm0 a, xmm1 b
asm_fldiv PROC
    divsd xmm0, xmm1       ; a / b
    ret
asm_fldiv ENDP

; double asm_flmod(double a, double b)
; asm_flmod xmm0 a, xmm1 b
asm_flmod PROC
    movapd xmm2, xmm0   ; xmm2 = a
    divsd xmm2, xmm1    ; xmm2 = a / b
    roundsd xmm2, xmm2, 3  ; truncate toward zero (SSE4.1)
    mulsd xmm2, xmm1    ; xmm2 = trunc(a/b) * b
    subsd xmm0, xmm2    ; xmm0 = a - xmm2
    ret
asm_flmod ENDP

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