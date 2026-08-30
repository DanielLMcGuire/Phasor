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

#include <stdint.h>

// Phasor vmcore/native arithmetic

#ifdef __cplusplus
extern "C"
{
#endif
	/// @brief Native addition
	int64_t asm_iadd(int64_t a, int64_t b);
	double  asm_fladd(double a, double b);
	/// @brief Native subtraction
	int64_t asm_isub(int64_t a, int64_t b);
	double  asm_flsub(double a, double b);
	/// @brief Native multiplication
	int64_t asm_imul(int64_t a, int64_t b);
	double  asm_flmul(double a, double b);
	/// @brief Native negation
	double asm_flneg(double a);
	/// @brief Native division
	int64_t asm_idiv(int64_t a, int64_t b);
	double  asm_fldiv(double a, double b);
	/// @brief Native modulus
	int64_t asm_imod(int64_t a, int64_t b);
	double  asm_flmod(double a, double b);
	/// @brief Native square root
	double asm_sqrt(double a);
	/// @brief Native power
	double asm_pow(double a, double b);
	/// @brief Native natural logarithm
	double asm_log(double a);
	/// @brief Native exponential
	double asm_exp(double a);
	/// @brief Native sine
	double asm_sin(double a);
	/// @brief Native cosine
	double asm_cos(double a);
	/// @brief Native tangent
	double asm_tan(double a);
#ifdef __cplusplus
}
#endif
