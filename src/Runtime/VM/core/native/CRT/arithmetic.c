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
