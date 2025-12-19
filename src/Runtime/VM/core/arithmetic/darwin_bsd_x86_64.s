# core/arithmetic/darwin_bsd_x86_64.s
# Arithmetic ops for x86_64 (macOS/BSD)

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

    .extern sqrt
    .extern pow
    .extern log
    .extern exp
    .extern sin
    .extern cos
    .extern tan

asm_add:
    mov rax, rdi
    add rax, rsi
    ret

asm_sub:
    mov rax, rdi
    sub rax, rsi
    ret

asm_mul:
    mov rax, rdi
    imul rax, rsi
    ret

asm_neg:
    mov rax, rdi
    neg rax
    ret

asm_div:
    mov rax, rdi
    mov r8, rsi
    cqo
    idiv r8
    ret

asm_mod:
    mov rax, rdi
    mov r8, rsi
    cqo
    idiv r8
    mov rax, rdx
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
