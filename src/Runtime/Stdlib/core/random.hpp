#pragma once

// my tiny xorshift+ implementation
// (C) Daniel McGuire -- MIT License

#include <cstdint>

extern "C" void     PHASORstd_rand_seed(uint64_t s0, uint64_t s1);
extern "C" uint64_t PHASORstd_rand_next();
extern "C" double   PHASORstd_rand_next_double();
extern "C" int64_t  PHASORstd_rand_next_range(int64_t min, int64_t max);