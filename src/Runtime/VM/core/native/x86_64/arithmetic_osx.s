# Copyright 2026 Daniel McGuire
# Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
# Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
# or https://phasor.pages.dev/LICENSE.txt
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
.intel_syntax noprefix
# Phasor vmcore/native arithmetic -- x86_64 OSX
.text

.global _asm_iadd
.global _asm_isub
.global _asm_imul
.global _asm_ineg
.global _asm_idiv
.global _asm_imod
.global _asm_fladd
.global _asm_flsub
.global _asm_flmul
.global _asm_flneg
.global _asm_fldiv
.global _asm_flmod
.global _asm_sqrt
.global _asm_pow
.global _asm_log
.global _asm_exp
.global _asm_sin
.global _asm_cos
.global _asm_tan

.extern _sqrt
.extern _pow
.extern _log
.extern _exp
.extern _sin
.extern _cos
.extern _tan

_asm_iadd:
    mov rax, rdi
    add rax, rsi
    ret

_asm_isub:
    mov rax, rdi
    sub rax, rsi
    ret

_asm_imul:
    mov rax, rdi
    imul rax, rsi
    ret

_asm_ineg:
    mov rax, rdi
    neg rax
    ret

_asm_idiv:
    mov rax, rdi
    test rsi, rsi
    jz .div_zero

    cqo
    idiv rsi
    ret

.div_zero:
    xor rax, rax
    ret

_asm_imod:
    mov rax, rdi
    test rsi, rsi
    jz .mod_zero

    cqo
    idiv rsi
    mov rax, rdx
    ret

.mod_zero:
    xor rax, rax
    ret

_asm_fladd:
    addsd xmm0, xmm1
    ret

_asm_flsub:
    subsd xmm0, xmm1
    ret

_asm_flmul:
    mulsd xmm0, xmm1
    ret

_asm_flneg:
    xorpd xmm1, xmm1
    subsd xmm1, xmm0
    movsd xmm0, xmm1
    ret

_asm_fldiv:
    divsd xmm0, xmm1
    ret

_asm_flmod:
    movapd xmm2, xmm0

    xorpd xmm3, xmm3
    ucomisd xmm1, xmm3
    je .fmod_zero

    divsd xmm2, xmm1
    roundsd xmm2, xmm2, 3
    mulsd xmm2, xmm1
    subsd xmm0, xmm2
    ret

.fmod_zero:
    xorpd xmm0, xmm0
    ret

_asm_sqrt:
    sub rsp, 8
    call _sqrt
    add rsp, 8
    ret

_asm_pow:
    sub rsp, 8
    call _pow
    add rsp, 8
    ret

_asm_log:
    sub rsp, 8
    call _log
    add rsp, 8
    ret

_asm_exp:
    sub rsp, 8
    call _exp
    add rsp, 8
    ret

_asm_sin:
    sub rsp, 8
    call _sin
    add rsp, 8
    ret

_asm_cos:
    sub rsp, 8
    call _cos
    add rsp, 8
    ret

_asm_tan:
    sub rsp, 8
    call _tan
    add rsp, 8
    ret
