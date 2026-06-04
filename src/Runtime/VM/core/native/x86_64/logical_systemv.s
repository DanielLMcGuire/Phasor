.intel_syntax noprefix
.text

.global asm_inot
.global asm_iand
.global asm_ior
.global asm_ixor
.global asm_iequal
.global asm_inot_equal
.global asm_iless_than
.global asm_igreater_than
.global asm_iless_equal
.global asm_igreater_equal
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

asm_inot:
    xor rax, rax
    test rdi, rdi
    setz al
    ret

asm_iand:
    xor rax, rax
    test rdi, rdi
    jz 1f
    test rsi, rsi
    jz 1f
    mov rax, 1
1:
    ret

asm_ior:
    xor rax, rax
    test rdi, rdi
    jnz 1f
    test rsi, rsi
    jz 2f
1:
    mov rax, 1
2:
    ret

asm_ixor:
    xor rax, rax
    test rdi, rdi
    setnz al
    mov r8b, al
    xor eax, eax
    test rsi, rsi
    setnz al
    xor al, r8b
    and al, 1
    movzx rax, al
    ret

asm_iequal:
    xor rax, rax
    cmp rdi, rsi
    sete al
    ret

asm_inot_equal:
    xor rax, rax
    cmp rdi, rsi
    setne al
    ret

asm_iless_than:
    xor rax, rax
    cmp rdi, rsi
    setl al
    ret

asm_igreater_than:
    xor rax, rax
    cmp rdi, rsi
    setg al
    ret

asm_iless_equal:
    xor rax, rax
    cmp rdi, rsi
    setle al
    ret

asm_igreater_equal:
    xor rax, rax
    cmp rdi, rsi
    setge al
    ret

asm_flnot:
    xor rax, rax
    xorpd xmm1, xmm1
    ucomisd xmm0, xmm1
    sete al
    ret

asm_fland:
    xor rax, rax
    xorpd xmm2, xmm2
    ucomisd xmm0, xmm2
    setne al
    mov r8b, al

    ucomisd xmm1, xmm2
    setne al
    and al, r8b
    movzx rax, al
    ret

asm_flor:
    xor rax, rax
    xorpd xmm2, xmm2
    ucomisd xmm0, xmm2
    setne al
    mov r8b, al

    ucomisd xmm1, xmm2
    setne al
    or al, r8b
    movzx rax, al
    ret

asm_flxor:
    xor rax, rax
    xorpd xmm2, xmm2
    ucomisd xmm0, xmm2
    setne al
    mov r8b, al

    ucomisd xmm1, xmm2
    setne al
    xor al, r8b
    and al, 1
    movzx rax, al
    ret

asm_flequal:
    xor rax, rax
    ucomisd xmm0, xmm1
    sete al
    ret

asm_flnot_equal:
    xor rax, rax
    ucomisd xmm0, xmm1
    setne al
    ret

asm_flless_than:
    xor rax, rax
    ucomisd xmm0, xmm1
    setb al
    ret

asm_flgreater_than:
    xor rax, rax
    ucomisd xmm0, xmm1
    seta al
    ret

asm_flless_equal:
    xor rax, rax
    ucomisd xmm0, xmm1
    setbe al
    ret

asm_flgreater_equal:
    xor rax, rax
    ucomisd xmm0, xmm1
    setae al
    ret