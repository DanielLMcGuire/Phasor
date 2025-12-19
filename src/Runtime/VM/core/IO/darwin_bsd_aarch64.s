// IO/darwin_bsd_aarch64.s
// IO functions for macOS/BSD ARM64

    .text
    .global asm_print_stdout
    .global asm_print_stderr
    .global asm_system
    .extern write
    .extern system

# void asm_print_stdout(const char* s, int64_t len)
asm_print_stdout:
    mov x0, #1          # fd = stdout
    mov x1, x0          # buffer pointer (original x0)
    mov x2, x1          # length (original x1)
    bl write
    ret

# void asm_print_stderr(const char* s, int64_t len)
asm_print_stderr:
    mov x0, #2          # fd = stderr
    mov x1, x0          # buffer pointer
    mov x2, x1          # length
    bl write
    ret

# int64_t asm_system(const char* cmd)
asm_system:
    bl system
    ret
