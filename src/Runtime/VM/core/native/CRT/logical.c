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

#include "../logical.h"

// Phasor vmcore/native logical -- CRT

[[nodiscard]] [[nodiscard]] int64_t asm_inot(int64_t a)
{
	return a == 0 ? 1 : 0;
}

[[nodiscard]] int64_t asm_iand(int64_t a, int64_t b)
{
	return (a != 0 && b != 0) ? 1 : 0;
}

[[nodiscard]] int64_t asm_ior(int64_t a, int64_t b)
{
	return (a != 0 || b != 0) ? 1 : 0;
}

[[nodiscard]] int64_t asm_ixor(int64_t a, int64_t b)
{
	int a_bool = (a != 0);
	int b_bool = (b != 0);
	return (a_bool ^ b_bool) ? 1 : 0;
}

[[nodiscard]] int64_t asm_iequal(int64_t a, int64_t b)
{
	return (a == b) ? 1 : 0;
}

[[nodiscard]] int64_t asm_inot_equal(int64_t a, int64_t b)
{
	return (a != b) ? 1 : 0;
}

[[nodiscard]] int64_t asm_iless_than(int64_t a, int64_t b)
{
	return (a < b) ? 1 : 0;
}

[[nodiscard]] int64_t asm_igreater_than(int64_t a, int64_t b)
{
	return (a > b) ? 1 : 0;
}

[[nodiscard]] int64_t asm_iless_equal(int64_t a, int64_t b)
{
	return (a <= b) ? 1 : 0;
}

[[nodiscard]] int64_t asm_igreater_equal(int64_t a, int64_t b)
{
	return (a >= b) ? 1 : 0;
}

[[nodiscard]] int64_t asm_flnot(double a)
{
	return a == 0 ? 1 : 0;
}

[[nodiscard]] int64_t asm_fland(double a, double b)
{
	return (a != 0 && b != 0) ? 1 : 0;
}

[[nodiscard]] int64_t asm_flor(double a, double b)
{
	return (a != 0 || b != 0) ? 1 : 0;
}

[[nodiscard]] int64_t asm_flxor(double a, double b)
{
	int a_bool = (a != 0);
	int b_bool = (b != 0);
	return (a_bool ^ b_bool) ? 1 : 0;
}

[[nodiscard]] int64_t asm_flequal(double a, double b)
{
	return (a == b) ? 1 : 0;
}

[[nodiscard]] int64_t asm_flnot_equal(double a, double b)
{
	return (a != b) ? 1 : 0;
}

[[nodiscard]] int64_t asm_flless_than(double a, double b)
{
	return (a < b) ? 1 : 0;
}

[[nodiscard]] int64_t asm_flgreater_than(double a, double b)
{
	return (a > b) ? 1 : 0;
}

[[nodiscard]] int64_t asm_flless_equal(double a, double b)
{
	return (a <= b) ? 1 : 0;
}

[[nodiscard]] int64_t asm_flgreater_equal(double a, double b)
{
	return (a >= b) ? 1 : 0;
}
