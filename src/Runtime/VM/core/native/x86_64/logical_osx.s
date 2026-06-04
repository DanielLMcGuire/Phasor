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

_asm_inot:
    xor rax, rax
    test rdi, rdi
    setz al
    ret

_asm_iand:
    xor rax, rax
    test rdi, rdi
    jz 1f
    test rsi, rsi
    jz 1f
    mov rax, 1
1:
    ret

_asm_ior:
    xor rax, rax
    test rdi, rdi
    jnz 1f
    test rsi, rsi
    jz 2f
1:
    mov rax, 1
2:
    ret

_asm_ixor:
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

_asm_iequal:
    xor rax, rax
    cmp rdi, rsi
    sete al
    ret

_asm_inot_equal:
    xor rax, rax
    cmp rdi, rsi
    setne al
    ret

_asm_iless_than:
    xor rax, rax
    cmp rdi, rsi
    setl al
    ret

_asm_igreater_than:
    xor rax, rax
    cmp rdi, rsi
    setg al
    ret

_asm_iless_equal:
    xor rax, rax
    cmp rdi, rsi
    setle al
    ret

_asm_igreater_equal:
    xor rax, rax
    cmp rdi, rsi
    setge al
    ret

_asm_flnot:
    xor rax, rax
    xorpd xmm1, xmm1
    ucomisd xmm0, xmm1
    sete al
    ret

_asm_fland:
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

_asm_flor:
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

_asm_flxor:
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

_asm_flequal:
    xor rax, rax
    ucomisd xmm0, xmm1
    sete al
    ret

_asm_flnot_equal:
    xor rax, rax
    ucomisd xmm0, xmm1
    setne al
    ret

_asm_flless_than:
    xor rax, rax
    ucomisd xmm0, xmm1
    setb al
    ret

_asm_flgreater_than:
    xor rax, rax
    ucomisd xmm0, xmm1
    seta al
    ret

_asm_flless_equal:
    xor rax, rax
    ucomisd xmm0, xmm1
    setbe al
    ret

_asm_flgreater_equal:
    xor rax, rax
    ucomisd xmm0, xmm1
    setae al
    ret