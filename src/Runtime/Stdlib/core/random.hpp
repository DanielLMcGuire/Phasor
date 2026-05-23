#pragma once

// my tiny xorshift+ implementation
// (C) Daniel McGuire -- MIT License

#include <phsint.hpp>

extern "C" void     PHASORstd_rand_seed(u64 s0, u64 s1);
extern "C" u64      PHASORstd_rand_next();
extern "C" f64      PHASORstd_rand_next_double();
extern "C" i64      PHASORstd_rand_next_range(i64 min, i64 max);