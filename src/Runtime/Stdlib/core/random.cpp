#include "random.hpp"
#include <phsint.hpp>

// my tiny xorshift+ implementation
// (C) Daniel McGuire -- MIT License

static Phasor::u64 s[2];

void PHASORstd_rand_seed(Phasor::u64 s0, Phasor::u64 s1)
{
	s[0] = s0;
	s[1] = s1;
}

Phasor::u64 PHASORstd_rand_next()
{
	Phasor::u64 s1 = s[0];
	Phasor::u64 s0 = s[1];

	s[0] = s0;
	s1 ^= s1 << 23;
	s[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);

	return s[1] + s0;
}

Phasor::f64 PHASORstd_rand_next_double()
{
	return (PHASORstd_rand_next() >> 11) * (1.0 / (UINT64_C(1) << 53));
}

Phasor::i64 PHASORstd_rand_next_range(Phasor::i64 min, Phasor::i64 max)
{
	return min + (Phasor::i64)(PHASORstd_rand_next() % (Phasor::u64)(max - min + 1));
}