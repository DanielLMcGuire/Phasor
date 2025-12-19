// core/logical/darwin_bsd_x86_64.s
// Logical ops for x86_64 (macOS/BSD) System V ABI

    .text
    .global asm_not
    .global asm_and
    .global asm_or
    .global asm_xor
    .global asm_equal
    .global asm_not_equal
    .global asm_less_than
    .global asm_greater_than
    .global asm_less_equal
    .global asm_greater_equal

asm_not:
    xor rax, rax
    test rdi, rdi
    setz al
    ret

asm_and:
    xor rax, rax
    test rdi, rdi
    jz .done_and
    test rsi, rsi
    jz .done_and
    mov rax, 1
.done_and:
    ret

asm_or:
    xor rax, rax
    test rdi, rdi
    jnz .set_true_or
    test rsi, rsi
    jz .done_or
.set_true_or:
    mov rax, 1
.done_or:
    ret

asm_xor:
    xor rax, rax
    test rdi, rdi
    setnz al
    mov r8, rax
    xor rax, rax
    test rsi, rsi
    setnz al
    xor rax, r8
    ret

asm_equal:
    xor rax, rax
    cmp rdi, rsi
    sete al
    ret

asm_not_equal:
    xor rax, rax
    cmp rdi, rsi
    setne al
    ret

asm_less_than:
    xor rax, rax
    cmp rdi, rsi
    setl al
    ret

asm_greater_than:
    xor rax, rax
    cmp rdi, rsi
    setg al
    ret

asm_less_equal:
    xor rax, rax
    cmp rdi, rsi
    setle al
    ret

asm_greater_equal:
    xor rax, rax
    cmp rdi, rsi
    setge al
    ret
