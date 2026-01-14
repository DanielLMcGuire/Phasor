; core/logical/windows_x86_64.asm
; Defines logical operations used by the VM for x86_64 on Windows

; PUBLIC SYMBOLS
PUBLIC asm_inot
PUBLIC asm_iand
PUBLIC asm_ior
PUBLIC asm_ixor
PUBLIC asm_iequal
PUBLIC asm_inot_equal
PUBLIC asm_iless_than
PUBLIC asm_igreater_than
PUBLIC asm_iless_equal
PUBLIC asm_igreater_equal
PUBLIC asm_flnot
PUBLIC asm_fland
PUBLIC asm_flor
PUBLIC asm_flxor
PUBLIC asm_flequal
PUBLIC asm_flnot_equal
PUBLIC asm_flless_than
PUBLIC asm_flgreater_than
PUBLIC asm_flless_equal
PUBLIC asm_flgreater_equal

.CODE

; int64_t asm_inot(int64_t a)
; asm_inot rcx a
asm_inot PROC
    xor rax, rax      ; clear rax
    test rcx, rcx     ; check if a == 0
    setz al           ; set rax = 1 if zero, 0 otherwise
    ret
asm_inot ENDP

; int64_t asm_iand(int64_t a, int64_t b)
; asm_iand rcx a, rdx b
asm_iand PROC
    xor rax, rax      ; initialize result = 0
    test rcx, rcx     ; check a
    jz done_and       ; short-circuit if a == 0
    test rdx, rdx     ; check b
    jz done_and       ; short-circuit if b == 0
    mov rax, 1        ; both non-zero -> result = 1
done_and:
    ret
asm_iand ENDP

; int64_t asm_ior(int64_t a, int64_t b)
; asm_ior rcx a, rdx b
asm_ior PROC
    xor rax, rax      ; initialize result = 0
    test rcx, rcx     ; check a
    jnz set_true_or   ; if a != 0, result = 1
    test rdx, rdx     ; check b
    jz done_or        ; if b == 0, result = 0
set_true_or:
    mov rax, 1        ; either a or b non-zero -> result = 1
done_or:
    ret
asm_ior ENDP

; int64_t asm_ixor(int64_t a, int64_t b)
; asm_ixor rcx a, rdx b
asm_ixor PROC
    xor rax, rax              ; clear result
    test rcx, rcx             ; check a
    setnz al                  ; set al = 1 if a != 0
    mov r8, rax               ; store a's truth value in r8
    xor rax, rax              ; clear rax again
    test rdx, rdx             ; check b
    setnz al                  ; set al = 1 if b != 0
    xor rax, r8               ; rax = a_truth_value XOR b_truth_value
    ret
asm_ixor ENDP

; int64_t asm_iequal(int64_t a, int64_t b)
; asm_iequal rcx a, rdx b
asm_iequal PROC
    xor rax, rax              ; clear result
    cmp rcx, rdx              ; compare a and b
    sete al                   ; set rax = 1 if equal
    ret
asm_iequal ENDP

; int64_t asm_inot_equal(int64_t a, int64_t b)
; asm_inot_equal rcx a, rdx b
asm_inot_equal PROC
    xor rax, rax
    cmp rcx, rdx
    setne al                  ; set rax = 1 if not equal
    ret
asm_inot_equal ENDP

; int64_t asm_iless_than(int64_t a, int64_t b)
; asm_iless_than rcx a, rdx b
asm_iless_than PROC
    xor rax, rax
    cmp rcx, rdx
    setl al                   ; set rax = 1 if a < b
    ret
asm_iless_than ENDP

; int64_t asm_igreater_than(int64_t a, int64_t b)
; asm_igreater_than rcx a, rdx b
asm_igreater_than PROC
    xor rax, rax
    cmp rcx, rdx
    setg al                   ; set rax = 1 if a > b
    ret
asm_igreater_than ENDP

; int64_t asm_iless_equal(int64_t a, int64_t b)
; asm_iless_equal rcx a, rdx b
asm_iless_equal PROC
    xor rax, rax
    cmp rcx, rdx
    setle al                  ; set rax = 1 if a <= b
    ret
asm_iless_equal ENDP

; int64_t asm_igreater_equal(int64_t a, int64_t b)
; asm_igreater_equal rcx a, rdx b
asm_igreater_equal PROC
    xor rax, rax
    cmp rcx, rdx
    setge al                  ; set rax = 1 if a >= b
    ret
asm_igreater_equal ENDP

; int64_t asm_flnot(double a)
; asm_flnot xmm0 a
asm_flnot PROC
    xor rax, rax
    xorpd xmm2, xmm2          ; zero xmm2
    ucomisd xmm0, xmm2        ; compare xmm0 with 0.0
    sete al                   ; rax = 1 if xmm0 == 0.0
    ret
asm_flnot ENDP

; int64_t asm_fland(double a, double b)
; asm_fland xmm0 a, xmm1 b
asm_fland PROC
    xor rax, rax
    xor r8, r8
    xor r9, r9
    xorpd xmm2, xmm2
    ucomisd xmm0, xmm2
    setne r8b                 ; r8 = xmm0 != 0
    ucomisd xmm1, xmm2
    setne r9b                 ; r9 = xmm1 != 0
    mov al, r8b
    and al, r9b               ; rax = xmm0 && xmm1
    ret
asm_fland ENDP

; int64_t asm_flor(double a, double b)
; asm_flor xmm0 a, xmm1 b
asm_flor PROC
    xor rax, rax
    xor r8, r8
    xor r9, r9
    xorpd xmm2, xmm2
    ucomisd xmm0, xmm2
    setne r8b
    ucomisd xmm1, xmm2
    setne r9b
    mov al, r8b
    or al, r9b                ; rax = xmm0 || xmm1
    ret
asm_flor ENDP

; int64_t asm_flxor(double a, double b)
; asm_flxor xmm0 a, xmm1 b
asm_flxor PROC
    xor rax, rax
    xor r8, r8
    xor r9, r9
    xorpd xmm2, xmm2
    ucomisd xmm0, xmm2
    setne r8b
    ucomisd xmm1, xmm2
    setne r9b
    mov al, r8b
    xor al, r9b               ; rax = xmm0 != xmm1
    ret
asm_flxor ENDP

; int64_t asm_flequal(double a, double b)
; asm_flequal xmm0 a, xmm1 b
asm_flequal PROC
    xor rax, rax
    ucomisd xmm0, xmm1
    sete al                   ; rax = 1 if xmm0 == xmm1
    ret
asm_flequal ENDP

; int64_t asm_flnot_equal(double a, double b)
; asm_flnot_equal xmm0 a, xmm1 b
asm_flnot_equal PROC
    xor rax, rax
    ucomisd xmm0, xmm1
    setne al                  ; rax = 1 if xmm0 != xmm1
    ret
asm_flnot_equal ENDP

; int64_t asm_flless_than(double a, double b)
; asm_flless_than xmm0 a, xmm1 b
asm_flless_than PROC
    xor rax, rax
    ucomisd xmm0, xmm1
    setb al                   ; rax = 1 if xmm0 < xmm1
    ret
asm_flless_than ENDP

; int64_t asm_flgreater_than(double a, double b)
; asm_flgreater_than xmm0 a, xmm1 b
asm_flgreater_than PROC
    xor rax, rax
    ucomisd xmm0, xmm1
    seta al                   ; rax = 1 if xmm0 > xmm1
    ret
asm_flgreater_than ENDP

; int64_t asm_flless_equal(double a, double b)
; asm_flless_equal xmm0 a, xmm1 b
asm_flless_equal PROC
    xor rax, rax
    ucomisd xmm0, xmm1
    setbe al                  ; rax = 1 if xmm0 <= xmm1
    ret
asm_flless_equal ENDP

; int64_t asm_flgreater_equal(double a, double b)
; asm_flgreater_equal xmm0 a, xmm1 b
asm_flgreater_equal PROC
    xor rax, rax
    ucomisd xmm0, xmm1
    setae al                  ; rax = 1 if xmm0 >= xmm1
    ret
asm_flgreater_equal ENDP


END