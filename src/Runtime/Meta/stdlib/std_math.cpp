// PULSAR STANDARD LIBRARY IMPLEMENTATION -- stdlib/std_math.cpp -- Standard Mathematical Operations
// Author: Daniel McGuire 

#include "stdlib.h"

int PULSARSTD_api::math::add(int a, int b) {
    return a + b;
}

int PULSARSTD_api::math::subtract(int a, int b) {
    return a - b;
}

int PULSARSTD_api::math::multiply(int a, int b) {
    return a * b;
}

int PULSARSTD_api::math::divide(int a, int b) {
    if (b == 0) {
        throw std::invalid_argument("Cannot divide by zero.");
    }
    return a / b;
}

int PULSARSTD_api::math::modulus(int a, int b) {
    if (b == 0) {
        throw std::invalid_argument("Cannot mod by zero.");
    }
    return a % b;
}

int PULSARSTD_api::math::power(int base, int exp) {
    if (exp < 0) {
        throw std::invalid_argument("Negative exponent not supported for int power.");
    }
    int result = 1;
    for (int i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}

int PULSARSTD_api::math::abs(int a) {
    return (a < 0) ? -a : a;
}

int PULSARSTD_api::math::min(int a, int b) {
    return (a < b) ? a : b;
}

int PULSARSTD_api::math::max(int a, int b) {
    return (a > b) ? a : b;
}

int PULSARSTD_api::math::factorial(int n) {
    if (n < 0) {
        throw std::invalid_argument("Factorial not defined for negative numbers.");
    }
    int result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}
int PULSARSTD_api::math::sqrt(int a) {
    if (a < 0) {
        throw std::invalid_argument("Cannot compute square root of negative number.");
    }
    int result = 0;
    while (result * result <= a) {
        result++;
    }
    return result - 1; 
}

