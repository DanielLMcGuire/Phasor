; core/IO/windows_aarch64.asm
; Defines input/output operations used by the VM for AArch64 on Windows

    AREA    |.text|, CODE, READONLY, ALIGN=2
    
    EXPORT  asm_print_stdout
    EXPORT  asm_print_stderr
    EXPORT  asm_system

    IMPORT GetStdHandle
    IMPORT WriteFile
    IMPORT system

; void asm_print_stdout(const char* s, int64_t len)
; asm_print_stdout x0 s, x1 len
asm_print_stdout
    stp x29, x30, [sp, #-48]! ; push frame pointer and return address, allocate stack
    mov x29, sp
    stp x19, x20, [sp, #16]   ; save callee-saved registers

    mov x19, x0               ; buffer pointer s
    mov x20, x1               ; length

    mov x0, #-11              ; STD_OUTPUT_HANDLE
    bl GetStdHandle            ; get handle to stdout

    mov x0, x0                ; hFile
    mov x1, x19               ; buffer
    mov x2, x20               ; number of bytes
    add x3, sp, #32           ; lpNumberOfBytesWritten
    mov x4, #0                ; lpOverlapped = NULL
    bl WriteFile

    ldp x19, x20, [sp, #16]   ; restore callee-saved registers
    ldp x29, x30, [sp], #48   ; restore frame pointer and return address, deallocate stack
    ret

; void asm_print_stderr(const char* s, int64_t len)
; asm_print_stderr x0 s, x1 len
asm_print_stderr
    stp x29, x30, [sp, #-48]! ; push frame pointer and return address, allocate stack
    mov x29, sp
    stp x19, x20, [sp, #16]   ; save callee-saved registers

    mov x19, x0               ; buffer pointer s
    mov x20, x1               ; length

    mov x0, #-12              ; STD_ERROR_HANDLE
    bl GetStdHandle           ; get handle to stdout

    mov x0, x0                ; hFile
    mov x1, x19               ; buffer
    mov x2, x20               ; number of bytes
    add x3, sp, #32           ; lpNumberOfBytesWritten
    mov x4, #0                ; lpOverlapped = NULL
    bl WriteFile

    ldp x19, x20, [sp, #16]   ; restore callee-saved registers
    ldp x29, x30, [sp], #48   ; restore frame pointer and return address, deallocate stack
    ret

; int64_t asm_system(const char* command)
; asm_system x0 command
asm_system
    bl system                 ; call system(command)
    ret
    END