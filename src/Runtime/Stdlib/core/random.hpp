// Copyright 2025-2026 Daniel McGuire
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

// Phasor stdlibcore random

#include <phsint.hpp>

extern "C" void        PHASORstd_rand_seed(Phasor::u64 s0, Phasor::u64 s1);
extern "C" Phasor::u64 PHASORstd_rand_next();
extern "C" Phasor::f64 PHASORstd_rand_next_double();
extern "C" Phasor::i64 PHASORstd_rand_next_range(Phasor::i64 min, Phasor::i64 max);
extern "C" Phasor::f64 PHASORstd_rand_next_double_range(Phasor::f64 min, Phasor::f64 max);