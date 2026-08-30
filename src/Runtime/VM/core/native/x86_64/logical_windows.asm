; Copyright 2025-2026 Daniel McGuire
; Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
; Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
; http://www.apache.org/licenses/LICENSE-2.0
; or https://phasor.pages.dev/LICENSE.txt
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.
; Phasor vmcore/native logical -- x86_64 WIN32
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

asm_inot PROC
    xor rax, rax
    test rcx, rcx
    setz al
    ret
asm_inot ENDP

asm_iand PROC
    xor rax, rax
    test rcx, rcx
    jz done
    test rdx, rdx
    jz done
    mov rax, 1
done:
    ret
asm_iand ENDP

asm_ior PROC
    xor rax, rax
    test rcx, rcx
    jnz true
    test rdx, rdx
    jz done
true:
    mov rax, 1
done:
    ret
asm_ior ENDP

asm_ixor PROC
    xor rax, rax
    test rcx, rcx
    setnz al
    mov r8b, al

    xor eax, eax
    test rdx, rdx
    setnz al

    xor al, r8b
    and al, 1
    movzx rax, al
    ret
asm_ixor ENDP

asm_iequal PROC
    xor rax, rax
    cmp rcx, rdx
    sete al
    ret
asm_iequal ENDP

asm_inot_equal PROC
    xor rax, rax
    cmp rcx, rdx
    setne al
    ret
asm_inot_equal ENDP

asm_iless_than PROC
    xor rax, rax
    cmp rcx, rdx
    setl al
    ret
asm_iless_than ENDP

asm_igreater_than PROC
    xor rax, rax
    cmp rcx, rdx
    setg al
    ret
asm_igreater_than ENDP

asm_iless_equal PROC
    xor rax, rax
    cmp rcx, rdx
    setle al
    ret
asm_iless_equal ENDP

asm_igreater_equal PROC
    xor rax, rax
    cmp rcx, rdx
    setge al
    ret
asm_igreater_equal ENDP

asm_flnot PROC
    xor rax, rax
    xorpd xmm2, xmm2
    ucomisd xmm0, xmm2
    sete al
    ret
asm_flnot ENDP

asm_fland PROC
    xor rax, rax
    xor r8, r8
    xor r9, r9
    xorpd xmm2, xmm2
    ucomisd xmm0, xmm2
    setne r8b
    ucomisd xmm1, xmm2
    setne r9b
    mov al, r8b
    and al, r9b
    movzx rax, al
    ret
asm_fland ENDP

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
    or al, r9b
    movzx rax, al
    ret
asm_flor ENDP

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
    xor al, r9b
    and al, 1
    movzx rax, al
    ret
asm_flxor ENDP

asm_flequal PROC
    xor rax, rax
    ucomisd xmm0, xmm1
    sete al
    ret
asm_flequal ENDP

asm_flnot_equal PROC
    xor rax, rax
    ucomisd xmm0, xmm1
    setne al
    ret
asm_flnot_equal ENDP

asm_flless_than PROC
    xor rax, rax
    ucomisd xmm0, xmm1
    setb al
    ret
asm_flless_than ENDP

asm_flgreater_than PROC
    xor rax, rax
    ucomisd xmm0, xmm1
    seta al
    ret
asm_flgreater_than ENDP

asm_flless_equal PROC
    xor rax, rax
    ucomisd xmm0, xmm1
    setbe al
    ret
asm_flless_equal ENDP

asm_flgreater_equal PROC
    xor rax, rax
    ucomisd xmm0, xmm1
    setae al
    ret
asm_flgreater_equal ENDP

END