// Copyright 2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with LLVM-Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://llvm.org/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// README
//
// Usage:
// ```cpp
// // #define NEED_PHASOR_C_ABI
// #include <Phasor.hpp>
// ```
//
// This header includes all Phasor headers required for using the runtime
// in C++ code. If you define NEED_PHASOR_C_ABI before including
// this header, it will also include the C ABI header.
//
// It is recommended to read the documentation via phasor-help CLI or at phasor-docs.pages.dev/man?f<TITLE>.<SECTION>
//
// Also read the below README docs in these headers:

#pragma once

#include "Phasor/PhasorFFI.hpp"
#include "Phasor/PhasorStdLib.hpp"
#include "Phasor/PhasorVM.hpp"
#include "Phasor/PhasorISA.hpp"
#include "Value.hpp"
#ifdef NEED_PHASOR_C_ABI
#include "PhasorRT.h"
#endif // PHASOR_C_ABI
