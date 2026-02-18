#pragma once
#include <cstdint>

static inline constexpr uint32_t ascii_to_u32_le(const char s[4])
{
	return ((uint32_t)(uint8_t)s[0]) | ((uint32_t)(uint8_t)s[1] << 8) | ((uint32_t)(uint8_t)s[2] << 16) |
	       ((uint32_t)(uint8_t)s[3] << 24);
}

/**
 * @brief Magic number (little endian)
 *
 * 'PHSB'
 */
#define MAGIC_NUMBER ascii_to_u32_le("PHSB")

/**
 * @brief Version number
 *
 * '3.0.0.0'
 */
const uint32_t VERSION = 0x03000000;