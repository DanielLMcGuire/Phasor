// Copyright 2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://phasor.pages.dev/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include <cstdint>

static constexpr uint32_t ascii_to_u32_le(const char s[4])
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