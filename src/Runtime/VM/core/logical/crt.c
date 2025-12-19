#include "logical.h"

int64_t asm_not(int64_t a) {
    return a == 0 ? 1 : 0;
}

int64_t asm_and(int64_t a, int64_t b) {
    return (a != 0 && b != 0) ? 1 : 0;
}

int64_t asm_or(int64_t a, int64_t b) {
    return (a != 0 || b != 0) ? 1 : 0;
}

int64_t asm_xor(int64_t a, int64_t b) {
    int a_bool = (a != 0);
    int b_bool = (b != 0);
    return (a_bool ^ b_bool) ? 1 : 0;
}

int64_t asm_equal(int64_t a, int64_t b) {
    return (a == b) ? 1 : 0;
}

int64_t asm_not_equal(int64_t a, int64_t b) {
    return (a != b) ? 1 : 0;
}

int64_t asm_less_than(int64_t a, int64_t b) {
    return (a < b) ? 1 : 0;
}

int64_t asm_greater_than(int64_t a, int64_t b) {
    return (a > b) ? 1 : 0;
}

int64_t asm_less_equal(int64_t a, int64_t b) {
    return (a <= b) ? 1 : 0;
}

int64_t asm_greater_equal(int64_t a, int64_t b) {
    return (a >= b) ? 1 : 0;
}
