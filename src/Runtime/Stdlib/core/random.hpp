#pragma once

// Phasor stdlibcore random
// MIT License
// (C) Daniel McGuire -- 2026

#include <phsint.hpp>

extern "C" void     PHASORstd_rand_seed(Phasor::u64 s0, Phasor::u64 s1);
extern "C" Phasor::u64      PHASORstd_rand_next();
extern "C" Phasor::f64      PHASORstd_rand_next_double();
extern "C" Phasor::i64      PHASORstd_rand_next_range(Phasor::i64 min, Phasor::i64 max);