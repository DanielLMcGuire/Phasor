.intel_syntax noprefix
.text

.global _asm_iadd
.global _asm_isub
.global _asm_imul
.global _asm_ineg
.global _asm_idiv
.global _asm_imod
.global _asm_fladd
.global _asm_flsub
.global _asm_flmul
.global _asm_flneg
.global _asm_fldiv
.global _asm_flmod
.global _asm_sqrt
.global _asm_pow
.global _asm_log
.global _asm_exp
.global _asm_sin
.global _asm_cos
.global _asm_tan

.extern _sqrt
.extern _pow
.extern _log
.extern _exp
.extern _sin
.extern _cos
.extern _tan

_asm_iadd:
    mov rax, rdi
    add rax, rsi
    ret

_asm_isub:
    mov rax, rdi
    sub rax, rsi
    ret

_asm_imul:
    mov rax, rdi
    imul rax, rsi
    ret

_asm_ineg:
    mov rax, rdi
    neg rax
    ret

_asm_idiv:
    mov rax, rdi
    cqo
    idiv rsi
    ret

_asm_imod:
    mov rax, rdi
    cqo
    idiv rsi
    mov rax, rdx
    ret

_asm_fladd:
    addsd xmm0, xmm1
    ret

_asm_flsub:
    subsd xmm0, xmm1
    ret

_asm_flmul:
    mulsd xmm0, xmm1
    ret

_asm_flneg:
    xorpd xmm1, xmm1
    subsd xmm1, xmm0
    movapd xmm0, xmm1
    ret

_asm_fldiv:
    divsd xmm0, xmm1
    ret

_asm_flmod:
    movapd xmm2, xmm0
    divsd xmm2, xmm1
    roundsd xmm2, xmm2, 3
    mulsd xmm2, xmm1
    subsd xmm0, xmm2
    ret

_asm_sqrt:
    sub rsp, 8
    call _sqrt
    add rsp, 8
    ret

_asm_pow:
    sub rsp, 8
    call _pow
    add rsp, 8
    ret

_asm_log:
    sub rsp, 8
    call _log
    add rsp, 8
    ret

_asm_exp:
    sub rsp, 8
    call _exp
    add rsp, 8
    ret

_asm_sin:
    sub rsp, 8
    call _sin
    add rsp, 8
    ret

_asm_cos:
    sub rsp, 8
    call _cos
    add rsp, 8
    ret

_asm_tan:
    sub rsp, 8
    call _tan
    add rsp, 8
    ret
