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

#include "../arithmetic.h"
#include <math.h>

// Phasor vmcore/native arithmetic -- CRT

[[nodiscard]] int64_t asm_iadd(int64_t a, int64_t b)
{
	return a + b;
}
[[nodiscard]] double asm_fladd(double a, double b)
{
	return a + b;
}

[[nodiscard]] int64_t asm_isub(int64_t a, int64_t b)
{
	return a - b;
}

[[nodiscard]] double asm_flsub(double a, double b)
{
	return a - b;
}

[[nodiscard]] int64_t asm_imul(int64_t a, int64_t b)
{
	return a * b;
}

[[nodiscard]] double asm_flmul(double a, double b)
{
	return a * b;
}

[[nodiscard]] int64_t asm_ineg(int64_t a)
{
	return -a;
}

[[nodiscard]] double asm_flneg(double a)
{
	return -a;
}

[[nodiscard]] int64_t asm_idiv(int64_t a, int64_t b)
{
	if (b == 0)
	{
		return 0;
	}
	return a / b;
}
[[nodiscard]] double asm_fldiv(double a, double b)
{
	return a / b;
}

[[nodiscard]] int64_t asm_imod(int64_t a, int64_t b)
{
	return a % b;
}

[[nodiscard]] double asm_flmod(double a, double b)
{
	return fmod(a, b);
}

[[nodiscard]] double asm_sqrt(double a)
{
	return sqrt(a);
}

[[nodiscard]] double asm_pow(double a, double b)
{
	return pow(a, b);
}

[[nodiscard]] double asm_log(double a)
{
	return log(a);
}

[[nodiscard]] double asm_exp(double a)
{
	return exp(a);
}

[[nodiscard]] double asm_sin(double a)
{
	return sin(a);
}

[[nodiscard]] double asm_cos(double a)
{
	return cos(a);
}

[[nodiscard]] double asm_tan(double a)
{
	return tan(a);
}
