#include <Value.hpp>
#include <utility>
#include "core/md5.h"
#include "core/base64.h"

#include "StdLib.hpp"

namespace Phasor
{

void StdLib::registerDataFunctions(VM *vm)
{
    vm->registerNativeFunction("base64_encode", base64_encoder);
    vm->registerNativeFunction("base64_decode", base64_decoder);
    vm->registerNativeFunction("md5", md5);
}

PhsString StdLib::base64_decoder(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 1, "base64_decode");
    requireString(args[0], "base64_decode", "base64 data");

    return base64::decode_string(args[0].string());
}

PhsString StdLib::base64_encoder(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 3, "base64_encode");
    requireString(args[0], "base64_encode", "input string");
    requireBool(args[1], "base64_encode", "url_safe");
    requireBool(args[2], "base64_encode", "pad");
    
    return base64::encode(args[0].string(), args[1].asBool(), args[2].asBool());
}

PhsString StdLib::md5(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 1, "md5");
    requireString(args[0], "md5", "input string");
    MD5 md5;
    return md5.hash(args[0].string());
}

}
