#pragma once

#include <array>
#include <cstddef>
#include <phsint.hpp>
#include <PhasorString.hpp>

class SHA1 {
public:
    static constexpr size_t kDigestSize = 20;
    static constexpr size_t kBlockSize  = 64;

    using Digest = std::array<Phasor::u8, kDigestSize>;

    SHA1();

    void update(const Phasor::u8* data, size_t len);
    void update(const Phasor::PhsString& data);

    const Digest& finalize();

    Phasor::PhsString finalizeHex();

    void reset();

    static Digest hash(const Phasor::u8* data, size_t len);
    static Digest hash(const Phasor::PhsString& data);

    static Phasor::PhsString hashHex(const Phasor::u8* data, size_t len);
    static Phasor::PhsString hashHex(const Phasor::PhsString& data);

    static Phasor::PhsString toHex(const Digest& digest);

private:
    void processBlock(const Phasor::u8 block[kBlockSize]);

    Phasor::u32 h_[5];
    Phasor::u64 bitLength_;
    Phasor::u8 buffer_[kBlockSize];
    size_t bufferLength_;
    bool finalized_;
    Digest digest_;
};
