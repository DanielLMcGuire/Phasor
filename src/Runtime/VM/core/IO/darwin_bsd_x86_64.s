// IO/darwin_bsd_x86_64.s
// IO functions for macOS/BSD x86_64

    .text
    .global asm_print_stdout
    .global asm_print_stderr
    .global asm_system
    .extern write
    .extern system

# void asm_print_stdout(const char* s, int64_t len)
asm_print_stdout:
    mov rdi, 1          # fd = stdout
    mov rsi, rdi        # buffer pointer (original RDI)
    mov rdx, rsi        # length (original RSI)
    call write
    ret

# void asm_print_stderr(const char* s, int64_t len)
asm_print_stderr:
    mov rdi, 2          # fd = stderr
    mov rsi, rdi        # buffer pointer
    mov rdx, rsi        # length
    call write
    ret

# int64_t asm_system(const char* cmd)
asm_system:
    call system
    ret
