#include "logical.h"

int64_t asm_inot(int64_t a)
{
	return a == 0 ? 1 : 0;
}

int64_t asm_iand(int64_t a, int64_t b)
{
	return (a != 0 && b != 0) ? 1 : 0;
}

int64_t asm_ior(int64_t a, int64_t b)
{
	return (a != 0 || b != 0) ? 1 : 0;
}

int64_t asm_ixor(int64_t a, int64_t b)
{
	int a_bool = (a != 0);
	int b_bool = (b != 0);
	return (a_bool ^ b_bool) ? 1 : 0;
}

int64_t asm_iequal(int64_t a, int64_t b)
{
	return (a == b) ? 1 : 0;
}

int64_t asm_inot_equal(int64_t a, int64_t b)
{
	return (a != b) ? 1 : 0;
}

int64_t asm_iless_than(int64_t a, int64_t b)
{
	return (a < b) ? 1 : 0;
}

int64_t asm_igreater_than(int64_t a, int64_t b)
{
	return (a > b) ? 1 : 0;
}

int64_t asm_iless_equal(int64_t a, int64_t b)
{
	return (a <= b) ? 1 : 0;
}

int64_t asm_igreater_equal(int64_t a, int64_t b)
{
	return (a >= b) ? 1 : 0;
}

int64_t asm_flnot(double a)
{
	return a == 0 ? 1 : 0;
}

int64_t asm_fland(double a, double b)
{
	return (a != 0 && b != 0) ? 1 : 0;
}

int64_t asm_flor(double a, double b)
{
	return (a != 0 || b != 0) ? 1 : 0;
}

int64_t asm_flxor(double a, double b)
{
	int a_bool = (a != 0);
	int b_bool = (b != 0);
	return (a_bool ^ b_bool) ? 1 : 0;
}

int64_t asm_flequal(double a, double b)
{
	return (a == b) ? 1 : 0;
}

int64_t asm_flnot_equal(double a, double b)
{
	return (a != b) ? 1 : 0;
}

int64_t asm_flless_than(double a, double b)
{
	return (a < b) ? 1 : 0;
}

int64_t asm_flgreater_than(double a, double b)
{
	return (a > b) ? 1 : 0;
}

int64_t asm_flless_equal(double a, double b)
{
	return (a <= b) ? 1 : 0;
}

int64_t asm_flgreater_equal(double a, double b)
{
	return (a >= b) ? 1 : 0;
}
