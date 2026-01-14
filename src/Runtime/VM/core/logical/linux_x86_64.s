# core/logical/linux_x86_64.s
# Defines logical operations used by the VM for x86_64 on Linux

    .text
    .globl asm_inot
    .globl asm_iand
    .globl asm_ior
    .globl asm_ixor
    .globl asm_iequal
    .globl asm_inot_equal
    .globl asm_iless_than
    .globl asm_igreater_than
    .globl asm_iless_equal
    .globl asm_igreater_equal
    .global asm_flnot
    .global asm_fland
    .global asm_flor
    .global asm_flxor
    .global asm_flequal
    .global asm_flnot_equal
    .global asm_flless_than
    .global asm_flgreater_than
    .global asm_flless_equal
    .global asm_flgreater_equal

# int64_t asm_inot(int64_t a)
asm_inot:
    xor rax, rax
    test rdi, rdi
    setz al
    ret

# int64_t asm_iand(int64_t a, int64_t b)
asm_iand:
    xor rax, rax
    test rdi, rdi
    jz .done_and
    test rsi, rsi
    jz .done_and
    mov rax, 1
.done_and:
    ret

# int64_t asm_ior(int64_t a, int64_t b)
asm_ior:
    xor rax, rax
    test rdi, rdi
    jnz .set_true_or
    test rsi, rsi
    jz .done_or
.set_true_or:
    mov rax, 1
.done_or:
    ret

# int64_t asm_ixor(int64_t a, int64_t b)
asm_ixor:
    xor rax, rax
    test rdi, rdi
    setnz al
    mov r8, rax
    xor rax, rax
    test rsi, rsi
    setnz al
    xor rax, r8
    ret

# int64_t asm_iequal(int64_t a, int64_t b)
asm_iequal:
    xor rax, rax
    cmp rdi, rsi
    sete al
    ret

# int64_t asm_inot_equal(int64_t a, int64_t b)
asm_inot_equal:
    xor rax, rax
    cmp rdi, rsi
    setne al
    ret

# int64_t asm_iless_than(int64_t a, int64_t b)
asm_iless_than:
    xor rax, rax
    cmp rdi, rsi
    setl al
    ret

# int64_t asm_igreater_than(int64_t a, int64_t b)
asm_igreater_than:
    xor rax, rax
    cmp rdi, rsi
    setg al
    ret

# int64_t asm_iless_equal(int64_t a, int64_t b)
asm_iless_equal:
    xor rax, rax
    cmp rdi, rsi
    setle al
    ret

# int64_t asm_igreater_equal(int64_t a, int64_t b)
asm_igreater_equal:
    xor rax, rax
    cmp rdi, rsi
    setge al
    ret

# int64_t asm_flnot(float a)
asm_flnot:
    xor rax, rax
    xorps xmm2, xmm2       # zero xmm2
    ucomiss xmm0, xmm2     # compare xmm0 with 0.0
    sete al                # al = 1 if xmm0 == 0.0
    ret

# int64_t asm_fland(float a, float b)
asm_fland:
    xor rax, rax
    xor r8, r8
    xor r9, r9
    xorps xmm2, xmm2
    ucomiss xmm0, xmm2
    setne r8b              # r8 = xmm0 != 0
    ucomiss xmm1, xmm2
    setne r9b              # r9 = xmm1 != 0
    mov al, r8b
    and al, r9b            # rax = xmm0 && xmm1
    ret

# int64_t asm_flor(float a, float b)
asm_flor:
    xor rax, rax
    xor r8, r8
    xor r9, r9
    xorps xmm2, xmm2
    ucomiss xmm0, xmm2
    setne r8b
    ucomiss xmm1, xmm2
    setne r9b
    mov al, r8b
    or al, r9b             # rax = xmm0 || xmm1
    ret

# int64_t asm_flxor(float a, float b)
asm_flxor:
    xor rax, rax
    xor r8, r8
    xor r9, r9
    xorps xmm2, xmm2
    ucomiss xmm0, xmm2
    setne r8b
    ucomiss xmm1, xmm2
    setne r9b
    mov al, r8b
    xor al, r9b            # rax = xmm0 != xmm1
    ret

# int64_t asm_flequal(float a, float b)
asm_flequal:
    xor rax, rax
    ucomiss xmm0, xmm1
    sete al
    ret

# int64_t asm_flnot_equal(float a, float b)
asm_flnot_equal:
    xor rax, rax
    ucomiss xmm0, xmm1
    setne al
    ret

# int64_t asm_flless_than(float a, float b)
asm_flless_than:
    xor rax, rax
    ucomiss xmm0, xmm1
    setb al                # below = <
    ret

# int64_t asm_flgreater_than(float a, float b)
asm_flgreater_than:
    xor rax, rax
    ucomiss xmm0, xmm1
    seta al                # above = >
    ret

# int64_t asm_flless_equal(float a, float b)
asm_flless_equal:
    xor rax, rax
    ucomiss xmm0, xmm1
    setbe al               # below or equal
    ret

# int64_t asm_flgreater_equal(float a, float b)
asm_flgreater_equal:
    xor rax, rax
    ucomiss xmm0, xmm1
    setae al               # above or equal
    ret

# int64_t asm_flnot(double a)
asm_flnot:
    xor rax, rax
    xorpd xmm2, xmm2       # zero xmm2
    ucomisd xmm0, xmm2     # compare xmm0 with 0.0
    sete al
    ret

# int64_t asm_fland(double a, double b)
asm_fland:
    xor rax, rax
    xor r8, r8
    xor r9, r9
    xorpd xmm2, xmm2
    ucomisd xmm0, xmm2
    setne r8b
    ucomisd xmm1, xmm2
    setne r9b
    mov al, r8b
    and al, r9b
    ret

# int64_t asm_flor(double a, double b)
asm_flor:
    xor rax, rax
    xor r8, r8
    xor r9, r9
    xorpd xmm2, xmm2
    ucomisd xmm0, xmm2
    setne r8b
    ucomisd xmm1, xmm2
    setne r9b
    mov al, r8b
    or al, r9b
    ret

# int64_t asm_flxor(double a, double b)
asm_flxor:
    xor rax, rax
    xor r8, r8
    xor r9, r9
    xorpd xmm2, xmm2
    ucomisd xmm0, xmm2
    setne r8b
    ucomisd xmm1, xmm2
    setne r9b
    mov al, r8b
    xor al, r9b
    ret

# int64_t asm_flequal(double a, double b)
asm_flequal:
    xor rax, rax
    ucomisd xmm0, xmm1
    sete al
    ret

# int64_t asm_flnot_equal(double a, double b)
asm_flnot_equal:
    xor rax, rax
    ucomisd xmm0, xmm1
    setne al
    ret

# int64_t asm_flless_than(double a, double b)
asm_flless_than:
    xor rax, rax
    ucomisd xmm0, xmm1
    setb al
    ret

# int64_t asm_flgreater_than(double a, double b)
asm_flgreater_than:
    xor rax, rax
    ucomisd xmm0, xmm1
    seta al
    ret

# int64_t asm_flless_equal(double a, double b)
asm_flless_equal:
    xor rax, rax
    ucomisd xmm0, xmm1
    setbe al
    ret

# int64_t asm_flgreater_equal(double a, double b)
asm_flgreater_equal:
    xor rax, rax
    ucomisd xmm0, xmm1
    setae al
    ret