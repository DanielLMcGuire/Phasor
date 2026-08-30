// Copyright 2025-2026 Daniel McGuire
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

#include <Value.hpp>
#include <utility>
#include "core/md5.h"
#include "core/base64.h"
#include "core/sha1.h"
#include "core/sha256.h"

#include "StdLib.hpp"

namespace Phasor
{

void StdLib::registerDataFunctions(VM *vm)
{
    vm->registerNativeFunction("base64_encode", base64_encoder);
    vm->registerNativeFunction("base64_decode", base64_decoder);
    vm->registerNativeFunction("md5", md5);
    vm->registerNativeFunction("sha1", sha1);
    vm->registerNativeFunction("sha256", sha256);

    vm->registerNativeFunction("phs__base64_encode_native", base64_encoder_native);
    vm->registerNativeFunction("phs__base64_decode_native", base64_decoder_native);
    vm->registerNativeFunction("phs__md5_native", md5_native);
    vm->registerNativeFunction("phs__sha1_native", sha1_native);
    vm->registerNativeFunction("phs__sha256_native", sha256_native);
}

Phasor::string StdLib::base64_decoder(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 1, "base64_decode");
    requireString(args[0], "base64_decode", "base64 data");

    return base64::decode_string(args[0].string());
}

Phasor::string StdLib::base64_encoder(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 2, "base64_encode");
    requireString(args[0], "base64_encode", "input string");
    requireBool(args[1], "base64_encode", "url_safe");
    
    return base64::encode(args[0].string(), args[1].asBool());
}

Phasor::string StdLib::md5(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 1, "md5");
    requireString(args[0], "md5", "input string");
    MD5 md5;
    return md5.hashHex(args[0].string());
}

Phasor::string StdLib::sha1(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 1, "sha1");

    SHA1 sha1;
    Value input = args[0];
    if (input.isString()) return sha1.hashHex(args[0].string());
    else if (input.isInt()) {
        i64 inputInteger = args[0].asInt();
        return sha1.hashHex(reinterpret_cast<const Phasor::u8*>(&inputInteger), sizeof(inputInteger));
    } else if (input.isFloat()) {
        f64 inputFloat = args[0].asFloat();
        return sha1.hashHex(reinterpret_cast<const Phasor::u8*>(&inputFloat), sizeof(inputFloat));
    }
    
    return "";
}

Phasor::string StdLib::sha256(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 1, "sha256");

    SHA256 sha256;
    Value input = args[0];
    if (input.isString()) return sha256.hashHex(args[0].string());
    else if (input.isInt()) {
        i64 inputInteger = args[0].asInt();
        return sha256.hashHex(reinterpret_cast<const Phasor::u8*>(&inputInteger), sizeof(inputInteger));
    } else if (input.isFloat()) {
        f64 inputFloat = args[0].asFloat();
        return sha256.hashHex(reinterpret_cast<const Phasor::u8*>(&inputFloat), sizeof(inputFloat));
    }
    
    return "";
}

Phasor::string StdLib::base64_encoder_native(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 3, "base64_encode_native");
    i64 address = requireAddress(args[0], "base64_encode_native", "first argument (address)");
    i64 length  = requireLength(args[1], "base64_encode_native", "second argument (length)");
    requireBool(args[2], "base64_encode_native", "third argument (url_safe)");

    const auto *src = static_cast<const unsigned char *>(i64_to_pointer(address));
    return base64::encode(src, static_cast<size_t>(length), args[2].asBool());
}

i64 StdLib::base64_decoder_native(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 3, "base64_decode_native");
    requireString(args[0], "base64_decode_native", "first argument (base64 data)");
    i64 address   = requireAddress(args[1], "base64_decode_native", "second argument (destination address)");
    i64 maxLength = requireLength(args[2], "base64_decode_native", "third argument (destination buffer size)");

    std::vector<unsigned char> decoded = base64::decode(args[0].string());

    if (std::cmp_greater(decoded.size(), maxLength))
        PHS_ERROR("base64_decode_native(): decoded data (" + std::to_string(decoded.size()) +
                  " bytes) does not fit in the destination buffer (" + std::to_string(maxLength) + " bytes)");

    auto *dst = static_cast<unsigned char *>(i64_to_pointer(address));
    std::memcpy(dst, decoded.data(), decoded.size());

    return static_cast<i64>(decoded.size());
}

Phasor::string StdLib::md5_native(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 2, "md5_native");
    i64 address = requireAddress(args[0], "md5_native", "first argument (address)");
    i64 length  = requireLength(args[1], "md5_native", "second argument (length)");

    const auto *src = static_cast<const Phasor::u8 *>(i64_to_pointer(address));
    MD5 md5;
    md5.update(src, static_cast<size_t>(length));
    return MD5::toHex(md5.finalize());
}

Phasor::string StdLib::sha1_native(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 2, "sha1_native");
    i64 address = requireAddress(args[0], "sha1_native", "first argument (address)");
    i64 length  = requireLength(args[1], "sha1_native", "second argument (length)");

    const auto *src = static_cast<const Phasor::u8 *>(i64_to_pointer(address));
    return SHA1::hashHex(src, static_cast<size_t>(length));
}

Phasor::string StdLib::sha256_native(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 2, "sha256_native");
    i64 address = requireAddress(args[0], "sha256_native", "first argument (address)");
    i64 length  = requireLength(args[1], "sha256_native", "second argument (length)");

    const auto *src = static_cast<const Phasor::u8 *>(i64_to_pointer(address));
    return SHA256::hashHex(src, static_cast<size_t>(length));
}

}
