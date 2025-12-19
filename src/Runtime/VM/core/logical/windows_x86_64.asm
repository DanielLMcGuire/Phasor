; core/logical/windows_x86_64.asm
; Defines logical operations used by the VM for x86_64 on Windows

; PUBLIC SYMBOLS
PUBLIC asm_not
PUBLIC asm_and
PUBLIC asm_or
PUBLIC asm_xor
PUBLIC asm_equal
PUBLIC asm_not_equal
PUBLIC asm_less_than
PUBLIC asm_greater_than
PUBLIC asm_less_equal
PUBLIC asm_greater_equal

.CODE

; int64_t asm_not(int64_t a)
; asm_not rcx a
asm_not PROC
    xor rax, rax      ; clear rax
    test rcx, rcx     ; check if a == 0
    setz al           ; set rax = 1 if zero, 0 otherwise
    ret
asm_not ENDP

; int64_t asm_and(int64_t a, int64_t b)
; asm_and rcx a, rdx b
asm_and PROC
    xor rax, rax      ; initialize result = 0
    test rcx, rcx     ; check a
    jz done_and       ; short-circuit if a == 0
    test rdx, rdx     ; check b
    jz done_and       ; short-circuit if b == 0
    mov rax, 1        ; both non-zero -> result = 1
done_and:
    ret
asm_and ENDP

; int64_t asm_or(int64_t a, int64_t b)
; asm_or rcx a, rdx b
asm_or PROC
    xor rax, rax      ; initialize result = 0
    test rcx, rcx     ; check a
    jnz set_true_or   ; if a != 0, result = 1
    test rdx, rdx     ; check b
    jz done_or        ; if b == 0, result = 0
set_true_or:
    mov rax, 1        ; either a or b non-zero -> result = 1
done_or:
    ret
asm_or ENDP

; int64_t asm_xor(int64_t a, int64_t b)
; asm_xor rcx a, rdx b
asm_xor PROC
    xor rax, rax              ; clear result
    test rcx, rcx             ; check a
    setnz al                  ; set al = 1 if a != 0
    mov r8, rax               ; store a's truth value in r8
    xor rax, rax              ; clear rax again
    test rdx, rdx             ; check b
    setnz al                  ; set al = 1 if b != 0
    xor rax, r8               ; rax = a_truth_value XOR b_truth_value
    ret
asm_xor ENDP

; int64_t asm_equal(int64_t a, int64_t b)
; asm_equal rcx a, rdx b
asm_equal PROC
    xor rax, rax              ; clear result
    cmp rcx, rdx              ; compare a and b
    sete al                   ; set rax = 1 if equal
    ret
asm_equal ENDP

; int64_t asm_not_equal(int64_t a, int64_t b)
; asm_not_equal rcx a, rdx b
asm_not_equal PROC
    xor rax, rax
    cmp rcx, rdx
    setne al                  ; set rax = 1 if not equal
    ret
asm_not_equal ENDP

; int64_t asm_less_than(int64_t a, int64_t b)
; asm_less_than rcx a, rdx b
asm_less_than PROC
    xor rax, rax
    cmp rcx, rdx
    setl al                   ; set rax = 1 if a < b
    ret
asm_less_than ENDP

; int64_t asm_greater_than(int64_t a, int64_t b)
; asm_greater_than rcx a, rdx b
asm_greater_than PROC
    xor rax, rax
    cmp rcx, rdx
    setg al                   ; set rax = 1 if a > b
    ret
asm_greater_than ENDP

; int64_t asm_less_equal(int64_t a, int64_t b)
; asm_less_equal rcx a, rdx b
asm_less_equal PROC
    xor rax, rax
    cmp rcx, rdx
    setle al                  ; set rax = 1 if a <= b
    ret
asm_less_equal ENDP

; int64_t asm_greater_equal(int64_t a, int64_t b)
; asm_greater_equal rcx a, rdx b
asm_greater_equal PROC
    xor rax, rax
    cmp rcx, rdx
    setge al                  ; set rax = 1 if a >= b
    ret
asm_greater_equal ENDP

END