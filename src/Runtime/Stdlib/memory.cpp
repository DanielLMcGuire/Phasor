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

#include <utility>

#include "StdLib.hpp"

namespace Phasor
{

void StdLib::registerMemoryFunctions(VM *vm)
{
    // allocation
	vm->registerNativeFunction("phs__memory_malloc_native", StdLib::native_memory_malloc);
	vm->registerNativeFunction("phs__memory_calloc_native", StdLib::native_memory_calloc);
	vm->registerNativeFunction("phs__memory_realloc_native", StdLib::native_memory_realloc);
    vm->registerNativeFunction("phs__memory_free_native", StdLib::native_memory_free);
    vm->registerNativeFunction("phs__memory_stralloc_native", StdLib::native_memory_stralloc);
	vm->registerNativeFunction("phs__memory_strcpy_native", StdLib::native_memory_strcpy);
    // writing
	vm->registerNativeFunction("phs__memory_write_native", StdLib::native_memory_write);
	vm->registerNativeFunction("phs__memory_write_offset_native", StdLib::native_memory_write_offset);
	vm->registerNativeFunction("phs__memory_write_string_native", StdLib::native_memory_write_string);
	vm->registerNativeFunction("phs__memory_write_string_offset_native", StdLib::native_memory_write_string_offset);
    // reading
	vm->registerNativeFunction("phs__memory_read_native", StdLib::native_memory_read);
	vm->registerNativeFunction("phs__memory_read_offset_native", StdLib::native_memory_read_offset);
	vm->registerNativeFunction("phs__memory_read_string_native", StdLib::native_memory_read_string);
	vm->registerNativeFunction("phs__memory_read_string_offset_native", StdLib::native_memory_read_string_offset);
}

#pragma region allocation

i64 StdLib::native_memory_malloc(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "malloc_native");

	i64 length = requireLength(args[0], "malloc_native", "first argument (length)");

	void* ptr = std::malloc(static_cast<std::size_t>(length));

	if (ptr == nullptr)
		PHS_ERROR("malloc_native(): malloc failed");

	return pointer_to_i64(ptr);
}

i64 StdLib::native_memory_calloc(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "calloc_native");

	i64 length = requireLength(args[0], "calloc_native", "first argument (length)");

	void* ptr = std::calloc(1, static_cast<std::size_t>(length));

	if (ptr == nullptr)
		PHS_ERROR("calloc_native(): calloc failed");

	return pointer_to_i64(ptr);
}

i64 StdLib::native_memory_realloc(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "realloc_native");

	requireInt(args[0], "realloc_native", "first argument (pointer)");
	i64 ptrValue = args[0].asInt();
	i64 length = requireLength(args[1], "realloc_native", "second argument (length)");

	void* oldPtr = i64_to_pointer(ptrValue);

	void* newPtr = std::realloc(oldPtr, static_cast<std::size_t>(length));

	if (newPtr == nullptr)
		PHS_ERROR("realloc_native(): realloc failed");

	return pointer_to_i64(newPtr);
}

Value StdLib::native_memory_free(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "free_native");

	requireAddress(args[0], "free_native", "first argument (pointer)");
	i64 address = args[0].asInt();

	void* ptr = i64_to_pointer(address);

	std::free(ptr);

	return phsnull;
}

Value StdLib::native_memory_strcpy(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 4, "strcpy_native");

	i64 dstAddress = requireAddress(args[0], "strcpy_native", "first argument (destination pointer)");
	i64 dstSize    = requireLength(args[1], "strcpy_native", "second argument (destination size)");
	i64 srcAddress = requireAddress(args[2], "strcpy_native", "third argument (source pointer)");
	i64 srcSize    = requireLength(args[3], "strcpy_native", "fourth argument (source size)");

	const auto* src = static_cast<const char*>(i64_to_pointer(srcAddress));
	auto* dst = static_cast<char*>(i64_to_pointer(dstAddress));

	size_t srcLen = 0;
	while (std::cmp_less(srcLen ,srcSize) && src[srcLen] != '\0')
	{
		srcLen++;
	}

	if (std::cmp_greater_equal(srcLen ,srcSize))
		PHS_ERROR("strcpy_native(): source string is not null-terminated within the given source size");

	if (srcLen + 1 > static_cast<size_t>(dstSize))
		PHS_ERROR("strcpy_native(): destination buffer is too small to hold the source string");

	std::memcpy(dst, src, srcLen + 1);

	return phsnull;
}

Value StdLib::native_memory_stralloc(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "stralloc_native");

	requireString(args[0], "stralloc_native", "first argument");

	Phasor::string data = args[0].string();
	size_t bufSize = data.size() + 1;

	void* ptr = std::malloc(bufSize);
	if (ptr == nullptr)
		PHS_ERROR("stralloc_native(): malloc failed");

	std::memcpy(ptr, data.c_str(), bufSize);

	i64 address = pointer_to_i64(ptr);
	return Value::createArray({address, static_cast<i64>(bufSize)});
}

#pragma endregion
#pragma region writing

Value StdLib::native_memory_write(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 3, "write_native");

	i64 address = requireAddress(args[0], "write_native", "first argument (pointer)");
	i64 length  = requireLength(args[1], "write_native", "second argument (length)");
	requireInt(args[2], "write_native", "third argument (data)");
	i64 data = args[2].asInt();

	if (std::cmp_greater(length, sizeof(i64)))
	{
		length = sizeof(i64);
	}

	auto* dst = static_cast<std::uint8_t*>(i64_to_pointer(address));

	std::memcpy(dst, &data, length);

	return phsnull;
}

Value StdLib::native_memory_write_offset(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 4, "write_offset_native");

    i64 address = requireAddress(
        args[0],
        "write_offset_native",
        "first argument (pointer)"
    );

    requireInt(
        args[1],
        "write_offset_native",
        "second argument (offset)"
    );

    i64 offset = args[1].asInt();

    i64 length = requireLength(
        args[2],
        "write_offset_native",
        "third argument (length)"
    );

    requireInt(
        args[3],
        "write_offset_native",
        "fourth argument (data)"
    );

    if (offset < 0 || offset > length)
    {
        PHS_ERROR("write_offset_native(): offset is outside the buffer");
    }

    i64 data = args[3].asInt();

    i64 effectiveLength = length - offset;

    if (std::cmp_greater(effectiveLength, sizeof(i64)))
    {
        effectiveLength = sizeof(i64);
    }

    auto *dst = static_cast<std::uint8_t *>(
        i64_to_pointer(address)
    );

    dst += offset;

    std::memcpy(
        dst,
        &data,
        static_cast<std::size_t>(effectiveLength)
    );

    return phsnull;
}

Value StdLib::native_memory_write_string(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 3, "write_string_native");

	i64 address = requireAddress(args[0], "write_string_native", "first argument (pointer)");
	i64 length  = requireLength(args[1], "write_string_native", "second argument (length)");
	requireString(args[2], "write_string_native", "third argument (data)");
	Phasor::string data = args[2].string();

	auto* dst = static_cast<char*>(i64_to_pointer(address));

	size_t copyLength = std::min(
		static_cast<size_t>(length - 1),
		data.size()
	);

	std::memcpy(dst, data.c_str(), copyLength);

	dst[copyLength] = '\0';
	return phsnull;
}

Value StdLib::native_memory_write_string_offset(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 4, "write_string_offset_native");

    i64 address = requireAddress(
        args[0],
        "write_string_offset_native",
        "first argument (pointer)"
    );

    requireInt(
        args[1],
        "write_string_offset_native",
        "second argument (offset)"
    );

    i64 offset = args[1].asInt();

    i64 length = requireLength(
        args[2],
        "write_string_offset_native",
        "third argument (length)"
    );

    requireString(
        args[3],
        "write_string_offset_native",
        "fourth argument (data)"
    );

    if (offset < 0 || offset > length)
    {
        PHS_ERROR("write_string_offset_native(): offset is outside the buffer");
    }

    Phasor::string data = args[3].string();

    i64 available = length - offset;

    if (available == 0)
    {
        PHS_ERROR(
            "write_string_offset_native(): buffer has no space for a null terminator"
        );
    }

    auto *dst = static_cast<char *>(
        i64_to_pointer(address)
    );

    dst += offset;

    size_t copyLength = std::min(
        static_cast<size_t>(available - 1),
        data.size()
    );

    std::memcpy(
        dst,
        data.c_str(),
        copyLength
    );

    dst[copyLength] = '\0';

    return phsnull;
}

#pragma endregion
#pragma region reading

i64 StdLib::native_memory_read(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "read_native");

	i64 address = requireAddress(args[0], "read_native", "first argument (pointer)");
	i64 length  = requireLength(args[1], "read_native", "second argument (length)");

	if (std::cmp_greater(length, sizeof(i64)))
	{
		length = sizeof(i64);
	}

	auto* src = static_cast<std::uint8_t*>(i64_to_pointer(address));

	i64 data = 0;
	std::memcpy(&data, src, static_cast<std::size_t>(length));

	return data;
}

i64 StdLib::native_memory_read_offset(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 3, "read_native_offset");

    i64 address = requireAddress(
        args[0],
        "read_native_offset",
        "first argument (pointer)"
    );

    requireInt(
        args[1],
        "read_native_offset",
        "second argument (offset)"
    );

    i64 offset = args[1].asInt();

    i64 length = requireLength(
        args[2],
        "read_native_offset",
        "third argument (length)"
    );

    if (offset < 0 || offset > length)
    {
        PHS_ERROR("read_native_offset(): offset is outside the buffer");
    }

    i64 effectiveLength = length - offset;

    if (std::cmp_greater(effectiveLength, sizeof(i64)))
    {
        effectiveLength = sizeof(i64);
    }

    auto *src = static_cast<std::uint8_t *>(
        i64_to_pointer(address)
    );

    src += offset;

    i64 data = 0;

    std::memcpy(
        &data,
        src,
        static_cast<std::size_t>(effectiveLength)
    );

    return data;
}

Phasor::string StdLib::native_memory_read_string(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "read_string_native");

	i64 address = requireAddress(args[0], "read_string_native", "first argument (pointer)");
	i64 length  = requireLength(args[1], "read_string_native", "second argument (length)");

	const auto* src = static_cast<const char*>(i64_to_pointer(address));

	std::string result;
	result.reserve(static_cast<size_t>(length));

	for (i64 i = 0; i < length; i++)
	{
		if (src[i] == '\0') 
		{
			break;
		}

		result.push_back(src[i]);
	}

	return result;
}

Phasor::string StdLib::native_memory_read_string_offset(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 3, "read_string_native_offset");

    i64 address = requireAddress(
        args[0],
        "read_string_native_offset",
        "first argument (pointer)"
    );

    requireInt(
        args[1],
        "read_string_native_offset",
        "second argument (offset)"
    );

    i64 offset = args[1].asInt();

    i64 length = requireLength(
        args[2],
        "read_string_native_offset",
        "third argument (length)"
    );

    if (offset < 0 || offset > length)
    {
        PHS_ERROR(
            "read_string_native_offset(): offset is outside the buffer"
        );
    }

    i64 available = length - offset;

    const auto *src = static_cast<const char *>(
        i64_to_pointer(address)
    );

    src += offset;

    std::string result;
    result.reserve(static_cast<size_t>(available));

    for (i64 i = 0; i < available; ++i)
    {
        if (src[i] == '\0')
        {
            break;
        }

        result.push_back(src[i]);
    }

    return result;
}

#pragma endregion

i64 StdLib::native_memory_argv(const Value::ArrayInstance &args, VM *) {
	checkArgCount(args, 0, "phs__argv_ptr");
	return pointer_to_i64(&argv);
}

i64 StdLib::native_memory_argc(const Value::ArrayInstance &args, VM *){
	checkArgCount(args, 0, "phs__argc_ptr");
	return pointer_to_i64(&argc);
}

} // namespace Phasor
