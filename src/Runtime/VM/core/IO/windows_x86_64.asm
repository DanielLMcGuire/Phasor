; IO/windows_x86_64.asm
; Defines input/output operations used by the VM for x86_64 on Windows

; PUBLIC SYMBOLS
PUBLIC asm_print_stdout
PUBLIC asm_print_stderr
PUBLIC asm_system

EXTERN GetStdHandle: PROC
EXTERN WriteFile: PROC
EXTERN system: PROC

STD_OUTPUT_HANDLE EQU -11
STD_ERROR_HANDLE  EQU -12

.CODE

; void asm_print_stdout(const char* s, int64_t len)
; asm_print_stdout rcx s, rdx len
asm_print_stdout PROC
    push rbx
    push rsi
    push rdi
    sub rsp, 56               ; allocate shadow space and temporary space

    mov rsi, rcx              ; save buffer pointer
    mov rdi, rdx              ; save length


    mov rcx, STD_OUTPUT_HANDLE
    call GetStdHandle         ; get handle to stdout
    mov rbx, rax              ; save handle

    mov rcx, rbx              ; handle
    mov rdx, rsi              ; pointer to buffer
    mov r8, rdi               ; number of bytes
    lea r9, [rsp+40]          ; pointer to bytes written
    mov qword ptr [rsp+32], 0 ; lpOverlapped = NULL
    call WriteFile

    add rsp, 56               ; deallocate shadow space and temporary space
    pop rdi
    pop rsi
    pop rbx
    ret
asm_print_stdout ENDP

; void asm_print_stderr(const char* s, int64_t len)
; asm_print_stderr rcx s, rdx len
asm_print_stderr PROC
    push rbx
    push rsi
    push rdi
    sub rsp, 56               ; allocate shadow space and temporary space

    mov rsi, rcx              ; save buffer pointer
    mov rdi, rdx              ; save length


    mov rcx, STD_ERROR_HANDLE
    call GetStdHandle         ; get handle to stdout
    mov rbx, rax              ; save handle

    mov rcx, rbx              ; handle
    mov rdx, rsi              ; pointer to buffer
    mov r8, rdi               ; number of bytes
    lea r9, [rsp+40]          ; pointer to bytes written
    mov qword ptr [rsp+32], 0 ; lpOverlapped = NULL
    call WriteFile

    add rsp, 56               ; deallocate shadow space and temporary space
    pop rdi
    pop rsi
    pop rbx
    ret
asm_print_stderr ENDP

; int64_t asm_system(const char* cmd)
; asm_system rcx cmd
asm_system PROC
    sub rsp, 40           ; allocate shadow space + align stack
    call system           ; call C runtime system()
    add rsp, 40           ; restore stack
    ret
asm_system ENDP

END