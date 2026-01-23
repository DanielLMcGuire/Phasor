.intel_syntax noprefix
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

# int64_t asm_iadd(int64_t a, int64_t b)
# rdi = a, rsi = b
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
    cqo
    idiv rsi
    ret

# int64_t asm_imod(int64_t a, int64_t b)
asm_imod:
    mov rax, rdi
    cqo
    idiv rsi
    mov rax, rdx
    ret

# double asm_fladd(double a, double b)
# xmm0 = a, xmm1 = b
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
    xorpd xmm1, xmm1
    subsd xmm1, xmm0
    movapd xmm0, xmm1
    ret

# double asm_fldiv(double a, double b)
asm_fldiv:
    divsd xmm0, xmm1
    ret

# double asm_flmod(double a, double b)
asm_flmod:
    movapd xmm2, xmm0
    divsd xmm2, xmm1
    roundsd xmm2, xmm2, 3
    mulsd xmm2, xmm1
    subsd xmm0, xmm2
    ret

# double asm_sqrt(double a)
asm_sqrt:
    sub rsp, 8
    call sqrt
    add rsp, 8
    ret

# double asm_pow(double a, double b)
asm_pow:
    sub rsp, 8
    call pow
    add rsp, 8
    ret

# double asm_log(double a)
asm_log:
    sub rsp, 8
    call log
    add rsp, 8
    ret

# double asm_exp(double a)
asm_exp:
    sub rsp, 8
    call exp
    add rsp, 8
    ret

# double asm_sin(double a)
asm_sin:
    sub rsp, 8
    call sin
    add rsp, 8
    ret

# double asm_cos(double a)
asm_cos:
    sub rsp, 8
    call cos
    add rsp, 8
    ret

# double asm_tan(double a)
asm_tan:
    sub rsp, 8
    call tan
    add rsp, 8
    ret
