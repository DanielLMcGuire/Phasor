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

#include "sha1.h"

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
	#define PHS_SRC_LOC() std::format("SHA1::{}()", __func__)
#endif

#define PHS_ERROR(x) throw std::logic_error(std::format("\"{}\" thrown in {}", x, PHS_SRC_LOC()))

namespace {

inline Phasor::u32 leftRotate(Phasor::u32 value, Phasor::u32 bits) {
    return (value << bits) | (value >> (32 - bits));
}

} // namespace

SHA1::SHA1() {
    reset();
}

void SHA1::reset() {
    h_[0] = 0x67452301u;
    h_[1] = 0xEFCDAB89u;
    h_[2] = 0x98BADCFEu;
    h_[3] = 0x10325476u;
    h_[4] = 0xC3D2E1F0u;

    bitLength_ = 0;
    bufferLength_ = 0;
    finalized_ = false;
    digest_.fill(0);
}

void SHA1::update(const Phasor::u8* data, size_t len) {
    if (finalized_) {
        PHS_ERROR(std::format("{} called after finalize(); call reset() to reuse", __func__));
    }
    if (len == 0) return;

    bitLength_ += static_cast<Phasor::u64>(len) * 8;

    size_t i = 0;

    if (bufferLength_ > 0) {
        while (bufferLength_ < kBlockSize && i < len) {
            buffer_[bufferLength_++] = data[i++];
        }
        if (bufferLength_ == kBlockSize) {
            processBlock(buffer_);
            bufferLength_ = 0;
        }
    }

    while (i + kBlockSize <= len) {
        processBlock(data + i);
        i += kBlockSize;
    }

    while (i < len) {
        buffer_[bufferLength_++] = data[i++];
    }
}

void SHA1::update(const Phasor::string& data) {
    update(reinterpret_cast<const Phasor::u8*>(data.data()), data.size());
}

const SHA1::Digest& SHA1::finalize() {
    if (!finalized_) {
        const Phasor::u64 totalBits = bitLength_;

        buffer_[bufferLength_++] = 0x80;
        if (bufferLength_ == kBlockSize) {
            processBlock(buffer_);
            bufferLength_ = 0;
        }

        while (bufferLength_ != kBlockSize - 8) {
            buffer_[bufferLength_++] = 0x00;
            if (bufferLength_ == kBlockSize) {
                processBlock(buffer_);
                bufferLength_ = 0;
            }
        }

        for (int i = 7; i >= 0; --i) {
            buffer_[bufferLength_++] = static_cast<Phasor::u8>((totalBits >> (i * 8)) & 0xFF);
        }
        processBlock(buffer_);
        bufferLength_ = 0;

        for (int i = 0; i < 5; ++i) {
            digest_[i * 4 + 0] = static_cast<Phasor::u8>((h_[i] >> 24) & 0xFF);
            digest_[i * 4 + 1] = static_cast<Phasor::u8>((h_[i] >> 16) & 0xFF);
            digest_[i * 4 + 2] = static_cast<Phasor::u8>((h_[i] >> 8) & 0xFF);
            digest_[i * 4 + 3] = static_cast<Phasor::u8>(h_[i] & 0xFF);
        }

        finalized_ = true;
    }
    return digest_;
}

Phasor::string SHA1::finalizeHex() {
    return toHex(finalize());
}

void SHA1::processBlock(const Phasor::u8 block[kBlockSize]) {
    Phasor::u32 w[80];

    for (int i = 0; i < 16; ++i) {
        w[i] = (static_cast<Phasor::u32>(block[i * 4 + 0]) << 24) |
               (static_cast<Phasor::u32>(block[i * 4 + 1]) << 16) |
               (static_cast<Phasor::u32>(block[i * 4 + 2]) << 8) |
               (static_cast<Phasor::u32>(block[i * 4 + 3]));
    }
    for (int i = 16; i < 80; ++i) {
        w[i] = leftRotate(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    Phasor::u32 a = h_[0];
    Phasor::u32 b = h_[1];
    Phasor::u32 c = h_[2];
    Phasor::u32 d = h_[3];
    Phasor::u32 e = h_[4];

    for (int i = 0; i < 80; ++i) {
        Phasor::u32 f, k;
        if (i < 20) {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999u;
        } else if (i < 40) {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1u;
        } else if (i < 60) {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDCu;
        } else {
            f = b ^ c ^ d;
            k = 0xCA62C1D6u;
        }

        const Phasor::u32 temp = leftRotate(a, 5) + f + e + k + w[i];
        e = d;
        d = c;
        c = leftRotate(b, 30);
        b = a;
        a = temp;
    }

    h_[0] += a;
    h_[1] += b;
    h_[2] += c;
    h_[3] += d;
    h_[4] += e;
}

SHA1::Digest SHA1::hash(const Phasor::u8* data, size_t len) {
    SHA1 sha1;
    sha1.update(data, len);
    return sha1.finalize();
}

SHA1::Digest SHA1::hash(const Phasor::string& data) {
    return hash(reinterpret_cast<const Phasor::u8*>(data.data()), data.size());
}

Phasor::string SHA1::hashHex(const Phasor::u8* data, size_t len) {
    return toHex(hash(data, len));
}

Phasor::string SHA1::hashHex(const Phasor::string& data) {
    return toHex(hash(data));
}

Phasor::string SHA1::toHex(const Digest& digest) {
    static const char* hexChars = "0123456789abcdef";
    Phasor::string result;
    result.reserve(kDigestSize * 2);
    for (Phasor::u8 byte : digest) {
        result.push_back(hexChars[byte >> 4]);
        result.push_back(hexChars[byte & 0x0F]);
    }
    return result;
}
