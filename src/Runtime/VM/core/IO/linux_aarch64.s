// core/IO/linux_aarch64.s
// Defines IO operations for VM on Linux AArch64

    .text
    .global asm_print_stdout
    .global asm_print_stderr
    .global asm_system

// void asm_print_stdout(const char* s, int64_t len)
asm_print_stdout:
    mov x8, #64        // syscall number: write
    mov x0, #1         // fd = stdout
    mov x1, x0         // buffer pointer (x0 originally has s)
    mov x2, x1         // length (x1 originally has len)
    svc #0
    ret

// void asm_print_stderr(const char* s, int64_t len)
asm_print_stderr:
    mov x8, #64        // syscall number: write
    mov x0, #2         // fd = stderr
    mov x1, x0         // buffer pointer
    mov x2, x1         // length
    svc #0
    ret

// int64_t asm_system(const char* command)
asm_system:
    bl system
    ret
