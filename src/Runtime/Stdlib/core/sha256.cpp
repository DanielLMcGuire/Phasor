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

#include "sha256.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>

#ifdef PHASOR_USES_BOOST
    #ifdef _WIN32
    #define BOOST_STACKTRACE_USE_WINDBG
    #endif
	#include <boost/stacktrace.hpp>
	#include <boost/assert/source_location.hpp>
	#define PHS_SRC_LOC() std::format("{} @ {}:{}", BOOST_CURRENT_LOCATION.function_name(), BOOST_CURRENT_LOCATION.file_name(), BOOST_CURRENT_LOCATION.line())
#else
#include <utility>
	#define PHS_SRC_LOC() std::format("SHA256::{}()", __func__)
#endif

#define PHS_ERROR(x) throw std::logic_error(std::format("\"{}\" thrown in {}", x, PHS_SRC_LOC()))

namespace {

constexpr Phasor::u32 K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

inline Phasor::u32 rightRotate(Phasor::u32 value, Phasor::u32 bits) {
    return (value >> bits) | (value << (32 - bits));
}

}  // namespace

SHA256::SHA256() {
    reset();
}

void SHA256::reset() {
    state_[0] = 0x6a09e667;
    state_[1] = 0xbb67ae85;
    state_[2] = 0x3c6ef372;
    state_[3] = 0xa54ff53a;
    state_[4] = 0x510e527f;
    state_[5] = 0x9b05688c;
    state_[6] = 0x1f83d9ab;
    state_[7] = 0x5be0cd19;

    bufferLen_ = 0;
    totalLen_ = 0;
    finalized_ = false;
    digestCache_.fill(0);
    std::memset(buffer_, 0, kBlockSize);
}

void SHA256::update(const Phasor::u8* data, size_t len) {
    if (finalized_) {
        PHS_ERROR(std::format("{} called after finalize(); call reset() to reuse", __func__));
    }

    totalLen_ += len;

    if (bufferLen_ > 0) {
        size_t needed = kBlockSize - bufferLen_;
        size_t toCopy = std::min(needed, len);
        std::memcpy(buffer_ + bufferLen_, data, toCopy);
        bufferLen_ += toCopy;
        data += toCopy;
        len -= toCopy;

        if (bufferLen_ == kBlockSize) {
            processBlock(buffer_);
            bufferLen_ = 0;
        }
    }

    while (len >= kBlockSize) {
        processBlock(data);
        data += kBlockSize;
        len -= kBlockSize;
    }

    if (len > 0) {
        std::memcpy(buffer_, data, len);
        bufferLen_ = len;
    }
}

void SHA256::update(const Phasor::string& data) {
    update(reinterpret_cast<const Phasor::u8*>(data.data()), data.size());
}

void SHA256::update(const std::vector<Phasor::u8>& data) {
    update(data.data(), data.size());
}

void SHA256::processBlock(const Phasor::u8* block) {
    Phasor::u32 w[64];

    for (int i = 0; i < 16; ++i) {
        w[i] = (static_cast<Phasor::u32>(block[i * 4]) << 24) |
               (static_cast<Phasor::u32>(block[i * 4 + 1]) << 16) |
               (static_cast<Phasor::u32>(block[i * 4 + 2]) << 8) |
               (static_cast<Phasor::u32>(block[i * 4 + 3]));
    }

    for (int i = 16; i < 64; ++i) {
        Phasor::u32 s0 = rightRotate(w[i - 15], 7) ^ rightRotate(w[i - 15], 18) ^ (w[i - 15] >> 3);
        Phasor::u32 s1 = rightRotate(w[i - 2], 17) ^ rightRotate(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    Phasor::u32 a = state_[0], b = state_[1], c = state_[2], d = state_[3];
    Phasor::u32 e = state_[4], f = state_[5], g = state_[6], h = state_[7];

    for (int i = 0; i < 64; ++i) {
        Phasor::u32 S1 = rightRotate(e, 6) ^ rightRotate(e, 11) ^ rightRotate(e, 25);
        Phasor::u32 ch = (e & f) ^ (~e & g);
        Phasor::u32 temp1 = h + S1 + ch + K[i] + w[i];
        Phasor::u32 S0 = rightRotate(a, 2) ^ rightRotate(a, 13) ^ rightRotate(a, 22);
        Phasor::u32 maj = (a & b) ^ (a & c) ^ (b & c);
        Phasor::u32 temp2 = S0 + maj;

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    state_[0] += a;
    state_[1] += b;
    state_[2] += c;
    state_[3] += d;
    state_[4] += e;
    state_[5] += f;
    state_[6] += g;
    state_[7] += h;
}

const SHA256::Digest& SHA256::finalize() {
    if (!finalized_) {
        const Phasor::u64 bitLen = totalLen_ * 8;

        buffer_[bufferLen_++] = 0x80;

        if (bufferLen_ > kBlockSize - 8) {
            std::memset(buffer_ + bufferLen_, 0, kBlockSize - bufferLen_);
            processBlock(buffer_);
            bufferLen_ = 0;
        }

        std::memset(buffer_ + bufferLen_, 0, kBlockSize - 8 - bufferLen_);

        for (int i = 0; i < 8; ++i) {
            buffer_[kBlockSize - 1 - i] = static_cast<Phasor::u8>(bitLen >> (8 * i));
        }

        processBlock(buffer_);

        for (int i = 0; i < 8; ++i) {
            digestCache_[i * 4] = static_cast<Phasor::u8>(state_[i] >> 24);
            digestCache_[i * 4 + 1] = static_cast<Phasor::u8>(state_[i] >> 16);
            digestCache_[i * 4 + 2] = static_cast<Phasor::u8>(state_[i] >> 8);
            digestCache_[i * 4 + 3] = static_cast<Phasor::u8>(state_[i]);
        }

        finalized_ = true;
    }

    return digestCache_;
}

Phasor::string SHA256::finalizeHex() {
    return toHex(finalize());
}

SHA256::Digest SHA256::hash(const Phasor::u8* data, size_t len) {
    SHA256 sha;
    sha.update(data, len);
    return sha.finalize();
}

SHA256::Digest SHA256::hash(const Phasor::string& data) {
    return hash(reinterpret_cast<const Phasor::u8*>(data.data()), data.size());
}

Phasor::string SHA256::hashHex(const Phasor::u8* data, size_t len) {
    return toHex(hash(data, len));
}

Phasor::string SHA256::hashHex(const Phasor::string& data) {
    return toHex(hash(data));
}

Phasor::string SHA256::toHex(const Digest& digest) {
    static const char* hexChars = "0123456789abcdef";
    Phasor::string result;
    result.reserve(kDigestSize * 2);
    for (Phasor::u8 byte : digest) {
        result.push_back(hexChars[byte >> 4]);
        result.push_back(hexChars[byte & 0x0F]);
    }
    return result;
}