# core/arithmetic/darwin_bsd_x86_64.s
# Arithmetic ops for x86_64 (macOS/BSD)

    .text
    .global asm_iadd
    .global asm_isub
    .global asm_imul
    .global asm_ineg
    .global asm_idiv
    .global asm_imod
    .global asm_fladd
    .global asm_flsub
    .global asm_flmul
    .global asm_flneg
    .global asm_fldiv
    .global asm_flmod
    .global asm_sqrt
    .global asm_pow
    .global asm_log
    .global asm_exp
    .global asm_sin
    .global asm_cos
    .global asm_tan

    .extern sqrt
    .extern pow
    .extern log
    .extern exp
    .extern sin
    .extern cos
    .extern tan

asm_iadd:
    mov rax, rdi
    add rax, rsi
    ret

asm_isub:
    mov rax, rdi
    sub rax, rsi
    ret

asm_imul:
    mov rax, rdi
    imul rax, rsi
    ret

asm_ineg:
    mov rax, rdi
    neg rax
    ret

asm_idiv:
    mov rax, rdi
    mov r8, rsi
    cqo
    idiv r8
    ret

asm_imod:
    mov rax, rdi
    mov r8, rsi
    cqo
    idiv r8
    mov rax, rdx
    ret

asm_fladd:
    addsd xmm0, xmm1
    ret

asm_flsub:
    subsd xmm0, xmm1
    ret

asm_flmul:
    mulsd xmm0, xmm1
    ret

asm_flneg:
    xorpd xmm0, xmm0        # zero
    subsd xmm0, xmm1        # 0 - x
    ret

asm_fldiv:
    divsd xmm0, xmm1
    ret

asm_flmod:
    # compute remainder = x - trunc(x/y) * y
    # xmm0 = dividend, xmm1 = divisor
    divsd xmm2, xmm0, xmm1         # xmm2 = x/y
    # truncate toward zero
    roundsd xmm2, xmm2, 3          # imm8 3 = truncate (SSE4.1)
    mulsd xmm2, xmm1               # xmm2 = trunc(x/y) * y
    subsd xmm0, xmm2               # xmm0 = x - xmm2
    ret


asm_sqrt:
    call sqrt
    ret

asm_pow:
    call pow
    ret

asm_log:
    call log
    ret

asm_exp:
    call exp
    ret

asm_sin:
    call sin
    ret

asm_cos:
    call cos
    ret

asm_tan:
    call tan
    ret
