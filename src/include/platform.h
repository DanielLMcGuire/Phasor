// Copyright 2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://phasor.pages.dev/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#ifndef PLATFORM_OVERRIDE
#if defined(__ORBIS__) || defined(_GAMING_XBOX_XBOXONE)
#define PLATFORM_LEGACY_GAME
#elif defined(_GAMING_DESKTOP)
#define PLATFORM_DESKTOP_GAME
#elif defined(_GAMING_XBOX_SCARLETT) || defined(__PROSPERO__) || defined(NN_SDK_BUILD)
#define PLATFORM_MODERN_GAME
#else
#define PLATFORM_DESKTOP_STANDARD
#endif
#endif

#if defined(PLATFORM_LEGACY_GAME)
#define MAX_REGISTERS 16
#elif defined(PLATFORM_DESKTOP_GAME) || defined(PLATFORM_MODERN_GAME)
#define MAX_REGISTERS 24
#elif defined(PLATFORM_DESKTOP_STANDARD)
#define MAX_REGISTERS 48
#endif

#if defined(__x86_64__) || defined(_M_X64)
    #define TARGET_ARCH_X64
    #define PHS_IS_64
#elif defined(__i386__) || defined(_M_IX86)
    #define TARGET_ARCH_X86
    #define PHS_IS_32
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define TARGET_ARCH_ARM64
    #define PHS_IS_64
#elif defined(__arm__) || defined(_M_ARM)
    #define TARGET_ARCH_ARM
    #define PHS_IS_32
#else
    #error "Unsupported architecture"
#endif