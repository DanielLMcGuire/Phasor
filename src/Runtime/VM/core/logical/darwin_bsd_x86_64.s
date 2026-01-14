// core/logical/darwin_bsd_x86_64.s
// Logical ops for x86_64 (macOS/BSD) System V ABI

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
    jz .done_and
    test rsi, rsi
    jz .done_and
    mov rax, 1
.done_and:
    ret

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
    xorpd xmm2, xmm2       ; zero xmm2
    ucomisd xmm0, xmm2     ; compare xmm0 with 0.0 (double)
    sete al
    ret

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
