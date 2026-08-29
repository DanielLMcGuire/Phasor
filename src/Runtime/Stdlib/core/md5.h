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

#include <array>
#include <phsint.hpp>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <PhasorString.hpp>
#include <vector>

class MD5 {
public:
    MD5() { reset(); }

    void update(const Phasor::u8* data, size_t len) {
        totalLenBits_ += static_cast<Phasor::u64>(len) * 8ULL;
        buffer_.insert(buffer_.end(), data, data + len);

        size_t offset = 0;
        while (buffer_.size() - offset >= 64) {
            processBlock(&buffer_[offset]);
            offset += 64;
        }
        buffer_.erase(buffer_.begin(), buffer_.begin() + offset);
    }

    void update(const Phasor::string& s) {
        update(reinterpret_cast<const Phasor::u8*>(s.data()), s.size());
    }

    std::array<Phasor::u8, 16> finalize() {
        const Phasor::u64 bitLen = totalLenBits_;

        buffer_.push_back(0x80);
        while (buffer_.size() % 64 != 56) {
            buffer_.push_back(0x00);
        }
        for (int i = 0; i < 8; ++i) {
            buffer_.push_back(static_cast<Phasor::u8>((bitLen >> (8 * i)) & 0xFF));
        }

        for (size_t offset = 0; offset < buffer_.size(); offset += 64) {
            processBlock(&buffer_[offset]);
        }
        buffer_.clear();

        std::array<Phasor::u8, 16> digest{};
        const Phasor::u32 state[4] = {a0_, b0_, c0_, d0_};
        for (int i = 0; i < 4; ++i) {
            digest[i * 4 + 0] = static_cast<Phasor::u8>(state[i] & 0xFF);
            digest[i * 4 + 1] = static_cast<Phasor::u8>((state[i] >> 8) & 0xFF);
            digest[i * 4 + 2] = static_cast<Phasor::u8>((state[i] >> 16) & 0xFF);
            digest[i * 4 + 3] = static_cast<Phasor::u8>((state[i] >> 24) & 0xFF);
        }
        return digest;
    }

    static Phasor::string toHex(const std::array<Phasor::u8, 16>& digest) {
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (Phasor::u8 b : digest) {
            oss << std::setw(2) << static_cast<int>(b);
        }
        return oss.str();
    }

    static Phasor::string hashHex(const Phasor::string& input) {
        MD5 md5;
        md5.update(input);
        return toHex(md5.finalize());
    }

private:
    Phasor::u32 a0_, b0_, c0_, d0_;
    Phasor::u64 totalLenBits_ = 0;
    std::vector<Phasor::u8> buffer_;

    void reset() {
        a0_ = 0x67452301;
        b0_ = 0xefcdab89;
        c0_ = 0x98badcfe;
        d0_ = 0x10325476;
        totalLenBits_ = 0;
        buffer_.clear();
    }

    static Phasor::u32 leftrotate(Phasor::u32 x, Phasor::u32 c) {
        return (x << c) | (x >> (32 - c));
    }

    void processBlock(const Phasor::u8 block[64]) {
        static const Phasor::u32 s[64] = {
            7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
            5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
            4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
            6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

        static const Phasor::u32 K[64] = {
            0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf,
            0x4787c62a, 0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af,
            0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e,
            0x49b40821, 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
            0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8, 0x21e1cde6,
            0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
            0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122,
            0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
            0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039,
            0xe6db99e5, 0x1fa27cf8, 0xc4ac5665, 0xf4292244, 0x432aff97,
            0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d,
            0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
            0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

        Phasor::u32 M[16];
        for (int i = 0; i < 16; ++i) {
            M[i] = static_cast<Phasor::u32>(block[i * 4]) |
                   (static_cast<Phasor::u32>(block[i * 4 + 1]) << 8) |
                   (static_cast<Phasor::u32>(block[i * 4 + 2]) << 16) |
                   (static_cast<Phasor::u32>(block[i * 4 + 3]) << 24);
        }

        Phasor::u32 A = a0_, B = b0_, C = c0_, D = d0_;

        for (Phasor::u32 i = 0; i < 64; ++i) {
            Phasor::u32 F, g;
            if (i < 16) {
                F = (B & C) | (~B & D);
                g = i;
            } else if (i < 32) {
                F = (D & B) | (~D & C);
                g = (5 * i + 1) % 16;
            } else if (i < 48) {
                F = B ^ C ^ D;
                g = (3 * i + 5) % 16;
            } else {
                F = C ^ (B | ~D);
                g = (7 * i) % 16;
            }
            F = F + A + K[i] + M[g];
            A = D;
            D = C;
            C = B;
            B = B + leftrotate(F, s[i]);
        }

        a0_ += A;
        b0_ += B;
        c0_ += C;
        d0_ += D;
    }
};