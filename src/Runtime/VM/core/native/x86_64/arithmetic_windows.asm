; Defines arithmetic operations used by the VM for x86_64 on Windows

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

asm_iadd PROC
    mov rax, rcx
    add rax, rdx
    ret
asm_iadd ENDP

asm_isub PROC
    mov rax, rcx
    sub rax, rdx
    ret
asm_isub ENDP

asm_imul PROC
    mov rax, rcx
    imul rax, rdx
    ret
asm_imul ENDP

asm_ineg PROC
    mov rax, rcx
    neg rax
    ret
asm_ineg ENDP

asm_idiv PROC
    mov rax, rcx
    test rdx, rdx
    jz div_zero

    mov r8, rdx
    cqo
    idiv r8
    ret

div_zero:
    xor rax, rax
    ret
asm_idiv ENDP

asm_imod PROC
    mov rax, rcx
    test rdx, rdx
    jz mod_zero

    mov r8, rdx
    cqo
    idiv r8
    mov rax, rdx
    ret

mod_zero:
    xor rax, rax
    ret
asm_imod ENDP

asm_fladd PROC
    addsd xmm0, xmm1
    ret
asm_fladd ENDP

asm_flsub PROC
    subsd xmm0, xmm1
    ret
asm_flsub ENDP

asm_flmul PROC
    mulsd xmm0, xmm1
    ret
asm_flmul ENDP

asm_flneg PROC
    xorpd xmm1, xmm1
    subsd xmm0, xmm1
    ret
asm_flneg ENDP

asm_fldiv PROC
    divsd xmm0, xmm1
    ret
asm_fldiv ENDP

asm_flmod PROC
    movapd xmm2, xmm0

    xorpd xmm3, xmm3
    ucomisd xmm1, xmm3
    je fmod_zero

    divsd xmm2, xmm1
    roundsd xmm2, xmm2, 3
    mulsd xmm2, xmm1
    subsd xmm0, xmm2
    ret

fmod_zero:
    xorpd xmm0, xmm0
    ret
asm_flmod ENDP

asm_sqrt PROC
    sub rsp, 40
    call sqrt
    add rsp, 40
    ret
asm_sqrt ENDP

asm_pow PROC
    sub rsp, 40
    call pow
    add rsp, 40
    ret
asm_pow ENDP

asm_log PROC
    sub rsp, 40
    call log
    add rsp, 40
    ret
asm_log ENDP

asm_exp PROC
    sub rsp, 40
    call exp
    add rsp, 40
    ret
asm_exp ENDP

asm_sin PROC
    sub rsp, 40
    call sin
    add rsp, 40
    ret
asm_sin ENDP

asm_cos PROC
    sub rsp, 40
    call cos
    add rsp, 40
    ret
asm_cos ENDP

asm_tan PROC
    sub rsp, 40
    call tan
    add rsp, 40
    ret
asm_tan ENDP

END