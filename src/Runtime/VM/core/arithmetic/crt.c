#include "arithmetic.h"
#include <math.h>

int64_t asm_add(int64_t a, int64_t b) {
    return a + b;
}

int64_t asm_sub(int64_t a, int64_t b) {
    return a - b;
}

int64_t asm_mul(int64_t a, int64_t b) {
    return a * b;
}

int64_t asm_neg(int64_t a) {
    return -a;
}

int64_t asm_div(int64_t a, int64_t b) {
    return a / b;
}

int64_t asm_mod(int64_t a, int64_t b) {
    return a % b;
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
