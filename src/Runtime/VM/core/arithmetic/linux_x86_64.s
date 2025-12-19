# core/arithmetic/linux_x86_64.s
# Defines arithmetic operations for VM on Linux x86_64

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

# int64_t asm_add(int64_t a, int64_t b)
asm_add:
    mov rax, rdi
    add rax, rsi
    ret

# int64_t asm_sub(int64_t a, int64_t b)
asm_sub:
    mov rax, rdi
    sub rax, rsi
    ret

# int64_t asm_mul(int64_t a, int64_t b)
asm_mul:
    mov rax, rdi
    imul rax, rsi
    ret

# int64_t asm_neg(int64_t a)
asm_neg:
    mov rax, rdi
    neg rax
    ret

# int64_t asm_div(int64_t a, int64_t b)
asm_div:
    mov rax, rdi
    mov r8, rsi
    cqo
    idiv r8
    ret

# int64_t asm_mod(int64_t a, int64_t b)
asm_mod:
    mov rax, rdi
    mov r8, rsi
    cqo
    idiv r8
    mov rax, rdx
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
