# core/logical/linux_x86_64.s
# Defines logical operations used by the VM for x86_64 on Linux

    .text
    .globl asm_not
    .globl asm_and
    .globl asm_or
    .globl asm_xor
    .globl asm_equal
    .globl asm_not_equal
    .globl asm_less_than
    .globl asm_greater_than
    .globl asm_less_equal
    .globl asm_greater_equal

# int64_t asm_not(int64_t a)
asm_not:
    xor rax, rax
    test rdi, rdi
    setz al
    ret

# int64_t asm_and(int64_t a, int64_t b)
asm_and:
    xor rax, rax
    test rdi, rdi
    jz .done_and
    test rsi, rsi
    jz .done_and
    mov rax, 1
.done_and:
    ret

# int64_t asm_or(int64_t a, int64_t b)
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

# int64_t asm_xor(int64_t a, int64_t b)
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

# int64_t asm_equal(int64_t a, int64_t b)
asm_equal:
    xor rax, rax
    cmp rdi, rsi
    sete al
    ret

# int64_t asm_not_equal(int64_t a, int64_t b)
asm_not_equal:
    xor rax, rax
    cmp rdi, rsi
    setne al
    ret

# int64_t asm_less_than(int64_t a, int64_t b)
asm_less_than:
    xor rax, rax
    cmp rdi, rsi
    setl al
    ret

# int64_t asm_greater_than(int64_t a, int64_t b)
asm_greater_than:
    xor rax, rax
    cmp rdi, rsi
    setg al
    ret

# int64_t asm_less_equal(int64_t a, int64_t b)
asm_less_equal:
    xor rax, rax
    cmp rdi, rsi
    setle al
    ret

# int64_t asm_greater_equal(int64_t a, int64_t b)
asm_greater_equal:
    xor rax, rax
    cmp rdi, rsi
    setge al
    ret
