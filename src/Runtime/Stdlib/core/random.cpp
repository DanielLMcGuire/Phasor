#include "random.hpp"

// my tiny xorshift+ implementation
// (C) Daniel McGuire -- MIT License

static uint64_t s[2];

void PHASORstd_rand_seed(uint64_t s0, uint64_t s1)
{
	s[0] = s0;
	s[1] = s1;
}

uint64_t PHASORstd_rand_next()
{
	uint64_t s1 = s[0];
	uint64_t s0 = s[1];

	s[0] = s0;
	s1 ^= s1 << 23;
	s[1] = s1 ^ s0 ^ (s1 >> 18) ^ (s0 >> 5);

	return s[1] + s0;
}

double PHASORstd_rand_next_double()
{
	return (PHASORstd_rand_next() >> 11) * (1.0 / (UINT64_C(1) << 53));
}

int64_t PHASORstd_rand_next_range(int64_t min, int64_t max)
{
	return min + (int64_t)(PHASORstd_rand_next() % (uint64_t)(max - min + 1));
}