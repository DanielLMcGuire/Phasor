.intel_syntax noprefix
.text
.global _asm_inot
.global _asm_iand
.global _asm_ior
.global _asm_ixor
.global _asm_iequal
.global _asm_inot_equal
.global _asm_iless_than
.global _asm_igreater_than
.global _asm_iless_equal
.global _asm_igreater_equal
.global _asm_flnot
.global _asm_fland
.global _asm_flor
.global _asm_flxor
.global _asm_flequal
.global _asm_flnot_equal
.global _asm_flless_than
.global _asm_flgreater_than
.global _asm_flless_equal
.global _asm_flgreater_equal

# int64_t asm_inot(int64_t a)
# rdi = a
_asm_inot:
    xor rax, rax
    test rdi, rdi
    setz al
    ret

# int64_t asm_iand(int64_t a, int64_t b)
# rdi = a, rsi = b
_asm_iand:
    xor rax, rax
    test rdi, rdi
    jz L_and_done
    test rsi, rsi
    jz L_and_done
    mov rax, 1
L_and_done:
    ret

# int64_t asm_ior(int64_t a, int64_t b)
_asm_ior:
    xor rax, rax
    test rdi, rdi
    jnz L_or_true
    test rsi, rsi
    jz L_or_done
L_or_true:
    mov rax, 1
L_or_done:
    ret

# int64_t asm_ixor(int64_t a, int64_t b)
_asm_ixor:
    xor rax, rax
    test rdi, rdi
    setnz al
    mov rcx, rax
    xor rax, rax
    test rsi, rsi
    setnz al
    xor rax, rcx
    ret

# int64_t asm_iequal(int64_t a, int64_t b)
_asm_iequal:
    xor rax, rax
    cmp rdi, rsi
    sete al
    ret

# int64_t asm_inot_equal(int64_t a, int64_t b)
_asm_inot_equal:
    xor rax, rax
    cmp rdi, rsi
    setne al
    ret

# int64_t asm_iless_than(int64_t a, int64_t b)
_asm_iless_than:
    xor rax, rax
    cmp rdi, rsi
    setl al
    ret

# int64_t asm_igreater_than(int64_t a, int64_t b)
_asm_igreater_than:
    xor rax, rax
    cmp rdi, rsi
    setg al
    ret

# int64_t asm_iless_equal(int64_t a, int64_t b)
_asm_iless_equal:
    xor rax, rax
    cmp rdi, rsi
    setle al
    ret

# int64_t asm_igreater_equal(int64_t a, int64_t b)
_asm_igreater_equal:
    xor rax, rax
    cmp rdi, rsi
    setge al
    ret

# int64_t asm_flnot(double a)
# xmm0 = a
_asm_flnot:
    xor rax, rax
    xorpd xmm1, xmm1
    ucomisd xmm0, xmm1
    sete al
    ret

# int64_t asm_fland(double a, double b)
# xmm0 = a, xmm1 = b
_asm_fland:
    xor rax, rax
    xorpd xmm2, xmm2
    ucomisd xmm0, xmm2
    setne al
    ucomisd xmm1, xmm2
    setne cl
    and al, cl
    ret

# int64_t asm_flor(double a, double b)
_asm_flor:
    xor rax, rax
    xorpd xmm2, xmm2
    ucomisd xmm0, xmm2
    setne al
    ucomisd xmm1, xmm2
    setne cl
    or al, cl
    ret

# int64_t asm_flxor(double a, double b)
_asm_flxor:
    xor rax, rax
    xorpd xmm2, xmm2
    ucomisd xmm0, xmm2
    setne al
    ucomisd xmm1, xmm2
    setne cl
    xor al, cl
    ret

# int64_t asm_flequal(double a, double b)
_asm_flequal:
    xor rax, rax
    ucomisd xmm0, xmm1
    sete al
    ret

# int64_t asm_flnot_equal(double a, double b)
_asm_flnot_equal:
    xor rax, rax
    ucomisd xmm0, xmm1
    setne al
    ret

# int64_t asm_flless_than(double a, double b)
_asm_flless_than:
    xor rax, rax
    ucomisd xmm0, xmm1
    setb al
    ret

# int64_t asm_flgreater_than(double a, double b)
_asm_flgreater_than:
    xor rax, rax
    ucomisd xmm0, xmm1
    seta al
    ret

# int64_t asm_flless_equal(double a, double b)
_asm_flless_equal:
    xor rax, rax
    ucomisd xmm0, xmm1
    setbe al
    ret

# int64_t asm_flgreater_equal(double a, double b)
_asm_flgreater_equal:
    xor rax, rax
    ucomisd xmm0, xmm1
    setae al
    ret