# Copyright 2025-2026 Daniel McGuire
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
# Phasor vmcore/native arithmetic -- x86_64 SYSTEMV
.text

.global asm_iadd
.global asm_isub
.global asm_imul
.global asm_ineg
.global asm_idiv
.global asm_imod
.global asm_fladd
.global asm_flsub
.global asm_flmul
.global asm_flneg
.global asm_fldiv
.global asm_flmod
.global asm_sqrt
.global asm_pow
.global asm_log
.global asm_exp
.global asm_sin
.global asm_cos
.global asm_tan

.extern sqrt
.extern pow
.extern log
.extern exp
.extern sin
.extern cos
.extern tan

asm_iadd:
    mov rax, rdi
    add rax, rsi
    ret

asm_isub:
    mov rax, rdi
    sub rax, rsi
    ret

asm_imul:
    mov rax, rdi
    imul rax, rsi
    ret

asm_ineg:
    mov rax, rdi
    neg rax
    ret

asm_idiv:
    mov rax, rdi
    test rsi, rsi
    jz .div_zero

    cqo
    idiv rsi
    ret

.div_zero:
    xor rax, rax
    ret

asm_imod:
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

asm_fladd:
    addsd xmm0, xmm1
    ret

asm_flsub:
    subsd xmm0, xmm1
    ret

asm_flmul:
    mulsd xmm0, xmm1
    ret

asm_flneg:
    xorpd xmm1, xmm1
    subsd xmm1, xmm0
    movsd xmm0, xmm1
    ret

asm_fldiv:
    divsd xmm0, xmm1
    ret

asm_flmod:
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

asm_sqrt:
    sub rsp, 40
    call sqrt
    add rsp, 40
    ret

asm_pow:
    sub rsp, 40
    call pow
    add rsp, 40
    ret

asm_log:
    sub rsp, 40
    call log
    add rsp, 40
    ret

asm_exp:
    sub rsp, 40
    call exp
    add rsp, 40
    ret

asm_sin:
    sub rsp, 40
    call sin
    add rsp, 40
    ret

asm_cos:
    sub rsp, 40
    call cos
    add rsp, 40
    ret

asm_tan:
    sub rsp, 40
    call tan
    add rsp, 40
    ret
