# core/arithmetic/linux_x86_64.s
# Defines arithmetic operations for VM on Linux x86_64

    .text
    .global asm_iadd
    .global asm_isub
    .global asm_imul
    .global asm_ineg
    .global asm_idiv
    .global asm_imod
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

# int64_t asm_iadd(int64_t a, int64_t b)
asm_iadd:
    mov rax, rdi
    add rax, rsi
    ret

# int64_t asm_isub(int64_t a, int64_t b)
asm_isub:
    mov rax, rdi
    sub rax, rsi
    ret

# int64_t asm_imul(int64_t a, int64_t b)
asm_imul:
    mov rax, rdi
    imul rax, rsi
    ret

# int64_t asm_ineg(int64_t a)
asm_ineg:
    mov rax, rdi
    neg rax
    ret

# int64_t asm_idiv(int64_t a, int64_t b)
asm_idiv:
    mov rax, rdi
    mov r8, rsi
    cqo
    idiv r8
    ret

# int64_t asm_imod(int64_t a, int64_t b)
asm_imod:
    mov rax, rdi
    mov r8, rsi
    cqo
    idiv r8
    mov rax, rdx
    ret

# double asm_fladd(double a, double b)
asm_fladd:
    addsd xmm0, xmm1
    ret

# double asm_flsub(double a, double b)
asm_flsub:
    subsd xmm0, xmm1
    ret

# double asm_flmul(double a, double b)
asm_flmul:
    mulsd xmm0, xmm1
    ret

# double asm_flneg(double a)
asm_flneg:
    xorpd xmm1, xmm1   # zero
    subsd xmm0, xmm1   # xmm0 = 0 - a
    ret

# double asm_fldiv(double a, double b)
asm_fldiv:
    divsd xmm0, xmm1
    ret

# double asm_flmod(double a, double b)
asm_flmod:
    divsd xmm2, xmm0, xmm1     # xmm2 = a / b
    roundsd xmm2, xmm2, 3      # truncate toward zero
    mulsd xmm2, xmm1           # xmm2 = trunc(a/b) * b
    subsd xmm0, xmm2           # xmm0 = a - xmm2
    ret

# double asm_sqrt(double a)
asm_sqrt:
    call sqrt
    ret

# double asm_pow(double a, double b)
asm_pow:
    call pow
    ret

# double asm_log(double a)
asm_log:
    call log
    ret

# double asm_exp(double a)
asm_exp:
    call exp
    ret

# double asm_sin(double a)
asm_sin:
    call sin
    ret

# double asm_cos(double a)
asm_cos:
    call cos
    ret

# double asm_tan(double a)
asm_tan:
    call tan
    ret
