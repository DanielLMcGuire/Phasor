#include "arithmetic.h"
#include <math.h>

int64_t asm_iadd(int64_t a, int64_t b) {
    return a + b;
}
double asm_fladd(double a, double b)
{
	return a + b;
}

int64_t asm_isub(int64_t a, int64_t b) {
    return a - b;
}
double asm_flsub(double a, double b)
{
	return a - b;
}

int64_t asm_imul(int64_t a, int64_t b) {
    return a * b;
}
double asm_flmul(double a, double b)
{
	return a * b;
}

int64_t asm_ineg(int64_t a) {
    return -a;
}
int64_t asm_flneg(double a)
{
	return -a;
}

int64_t asm_idiv(int64_t a, int64_t b) {
    return a / b;
}
double asm_fldiv(double a, double b)
{
	return a / b;
}

int64_t asm_imod(int64_t a, int64_t b) {
    return a % b;
}
double asm_flmod(double a, double b)
{
	return fmod(a, b);
}

double asm_sqrt(double a) {
    return sqrt(a);
}

double asm_pow(double a, double b) {
    return pow(a, b);
}

double asm_log(double a) {
    return log(a);
}

double asm_exp(double a) {
    return exp(a);
}

double asm_sin(double a) {
    return sin(a);
}

double asm_cos(double a) {
    return cos(a);
}

double asm_tan(double a) {
    return tan(a);
}
