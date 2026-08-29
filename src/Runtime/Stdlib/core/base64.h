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
#include <cstdint>
#include <stdexcept>
#include <PhasorString.hpp>
#include <vector>

// Phasor stdlibcore data/base64

namespace base64 {

namespace detail {

inline const char* alphabet(bool url_safe) {
    static const char* standard_table =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static const char* url_table =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    return url_safe ? url_table : standard_table;
}

inline const std::array<int, 256>& reverse_table() {
    static const std::array<int, 256> table = [] {
        std::array<int, 256> t;
        t.fill(-1);
        const char* std_alpha = alphabet(false);
        const char* url_alpha = alphabet(true);
        for (int i = 0; i < 64; ++i) {
            t[static_cast<unsigned char>(std_alpha[i])] = i;
            t[static_cast<unsigned char>(url_alpha[i])] = i;
        }
        return t;
    }();
    return table;
}

} // namespace detail

inline Phasor::string encode(const unsigned char* data, std::size_t len,
                           bool url_safe = false, bool pad = true) {
    const char* table = detail::alphabet(url_safe);
    Phasor::string out;
    out.reserve(((len + 2) / 3) * 4);

    std::size_t i = 0;
    while (i + 3 <= len) {
        unsigned int n = (static_cast<unsigned int>(data[i]) << 16) |
                         (static_cast<unsigned int>(data[i + 1]) << 8) |
                          static_cast<unsigned int>(data[i + 2]);
        out.push_back(table[(n >> 18) & 0x3F]);
        out.push_back(table[(n >> 12) & 0x3F]);
        out.push_back(table[(n >> 6) & 0x3F]);
        out.push_back(table[n & 0x3F]);
        i += 3;
    }

    const std::size_t rem = len - i;
    if (rem == 1) {
        unsigned int n = static_cast<unsigned int>(data[i]) << 16;
        out.push_back(table[(n >> 18) & 0x3F]);
        out.push_back(table[(n >> 12) & 0x3F]);
        if (pad) { out.push_back('='); out.push_back('='); }
    } else if (rem == 2) {
        unsigned int n = (static_cast<unsigned int>(data[i]) << 16) |
                         (static_cast<unsigned int>(data[i + 1]) << 8);
        out.push_back(table[(n >> 18) & 0x3F]);
        out.push_back(table[(n >> 12) & 0x3F]);
        out.push_back(table[(n >> 6) & 0x3F]);
        if (pad) out.push_back('=');
    }

    return out;
}

inline Phasor::string encode(const std::vector<unsigned char>& data,
                           bool url_safe = false, bool pad = true) {
    return encode(data.data(), data.size(), url_safe, pad);
}

inline Phasor::string encode(const Phasor::string& data, bool url_safe = false,
                           bool pad = true) {
    return encode(reinterpret_cast<const unsigned char*>(data.data()),
                  data.size(), url_safe, pad);
}

inline std::vector<unsigned char> decode(const Phasor::string& in) {
    std::size_t len = in.size();
    while (len > 0 && in[len - 1] == '=') --len;

    if (in.size() % 4 != 0)
        throw std::invalid_argument(
            "base64::decode: input length must be a multiple of 4 "
            "(pad with '=' or use an unpadded-aware caller)");

    const auto& rt = detail::reverse_table();
    std::vector<unsigned char> out;
    out.reserve((in.size() / 4) * 3);

    for (std::size_t i = 0; i < in.size(); i += 4) {
        int v[4];
        int pad_count = 0;
        for (int j = 0; j < 4; ++j) {
            unsigned char c = static_cast<unsigned char>(in[i + j]);
            if (c == '=') {
                if (i + 4 != in.size() || (j != 2 && j != 3))
                    throw std::invalid_argument(
                        "base64::decode: unexpected '=' padding");
                v[j] = 0;
                ++pad_count;
            } else {
                if (pad_count > 0)
                    throw std::invalid_argument(
                        "base64::decode: data found after padding");
                int val = rt[c];
                if (val < 0)
                    throw std::invalid_argument(
                        "base64::decode: invalid base64 character");
                v[j] = val;
            }
        }

        unsigned int n = (static_cast<unsigned int>(v[0]) << 18) |
                         (static_cast<unsigned int>(v[1]) << 12) |
                         (static_cast<unsigned int>(v[2]) << 6) |
                          static_cast<unsigned int>(v[3]);

        out.push_back(static_cast<unsigned char>((n >> 16) & 0xFF));
        if (pad_count < 2) out.push_back(static_cast<unsigned char>((n >> 8) & 0xFF));
        if (pad_count < 1) out.push_back(static_cast<unsigned char>(n & 0xFF));
    }

    return out;
}

inline Phasor::string decode_string(const Phasor::string& in) {
    std::vector<unsigned char> bytes = decode(in);
    return Phasor::string(bytes.begin(), bytes.end());
}

} // namespace base64