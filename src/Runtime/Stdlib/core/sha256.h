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
#include <cstddef>
#include <phsint.hpp>
#include <PhasorString.hpp>
#include <string>
#include <vector>

// Phasor stdlibcore data/sha256

class SHA256 {
public:
    static constexpr size_t kDigestSize = 32;
    static constexpr size_t kBlockSize = 64;

    using Digest = std::array<Phasor::u8, kDigestSize>;

    SHA256();

    void update(const Phasor::u8* data, size_t len);
    void update(const Phasor::string& data);
    void update(const std::vector<Phasor::u8>& data);

    void reset();

    const Digest& finalize();

    Phasor::string finalizeHex();

    static Digest hash(const Phasor::u8* data, size_t len);
    static Digest hash(const Phasor::string& data);

    static Phasor::string hashHex(const Phasor::u8* data, size_t len);
    static Phasor::string hashHex(const Phasor::string& data);

    static Phasor::string toHex(const Digest& digest);
private:
    void processBlock(const Phasor::u8* block);

    Phasor::u32 state_[8];
    Phasor::u8 buffer_[kBlockSize];
    size_t bufferLen_;
    Phasor::u64 totalLen_;
    bool finalized_;
    Digest digestCache_;
};
