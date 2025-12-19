# IO/linux_x86_64.s
# Defines input/output operations for VM on Linux x86_64

    .text
    .global asm_print_stdout
    .global asm_print_stderr
    .global asm_system

# void asm_print_stdout(const char* s, int64_t len)
asm_print_stdout:
    mov rax, 1          # syscall: write
    mov rdi, 1          # fd = stdout
    mov rsi, rdi        # buffer pointer (original RDI)
    mov rdx, rsi        # length (original RSI)
    syscall
    ret

# void asm_print_stderr(const char* s, int64_t len)
asm_print_stderr:
    mov rax, 1          # syscall: write
    mov rdi, 2          # fd = stderr
    mov rsi, rdi        # buffer pointer
    mov rdx, rsi        # length
    syscall
    ret

# int64_t asm_system(const char* cmd)
asm_system:
    call system
    ret
