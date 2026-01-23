.intel_syntax noprefix
.text

.global asm_print_stdout
.global asm_print_stderr
.global asm_system

.extern system

# void asm_print_stdout(const char* s, int64_t len)
# rdi = s, rsi = len
asm_print_stdout:
    mov rax, 1          # sys_write
    mov rdx, rsi        # len
    mov rsi, rdi        # buf
    mov rdi, 1          # stdout fd
    syscall
    ret

# void asm_print_stderr(const char* s, int64_t len)
# rdi = s, rsi = len
asm_print_stderr:
    mov rax, 1          # sys_write
    mov rdx, rsi        # len
    mov rsi, rdi        # buf
    mov rdi, 2          # stderr fd
    syscall
    ret

# int64_t asm_system(const char* cmd)
# rdi = cmd
asm_system:
    sub rsp, 8          # stack alignment
    call system
    add rsp, 8
    ret
