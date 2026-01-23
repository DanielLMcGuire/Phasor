.intel_syntax noprefix
.text

.global _asm_print_stdout
.global _asm_print_stderr
.global _asm_system

.extern _write
.extern _system

# void asm_print_stdout(const char* s, int64_t len)
# rdi = s, rsi = len
_asm_print_stdout:
    mov rdx, rsi        # len
    mov rsi, rdi        # buf
    mov rdi, 1          # stdout fd
    call _write
    ret

# void asm_print_stderr(const char* s, int64_t len)
_asm_print_stderr:
    mov rdx, rsi        # len
    mov rsi, rdi        # buf
    mov rdi, 2          # stderr fd
    call _write
    ret

# int64_t asm_system(const char* cmd)
_asm_system:
    sub rsp, 8
    call _system
    add rsp, 8
    ret
