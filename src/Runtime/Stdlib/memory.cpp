#include <utility>

#include "StdLib.hpp"

namespace Phasor
{

void StdLib::registerMemoryFunctions(VM *vm)
{
	vm->registerNativeFunction("free", StdLib::var_free);
	vm->registerNativeFunction("phs__memory_malloc_native", StdLib::native_memory_malloc);
	vm->registerNativeFunction("phs__memory_calloc_native", StdLib::native_memory_calloc);
	vm->registerNativeFunction("phs__memory_realloc_native", StdLib::native_memory_realloc);
	vm->registerNativeFunction("phs__memory_stralloc_native", StdLib::native_memory_stralloc);
	vm->registerNativeFunction("phs__memory_strcpy_native", StdLib::native_memory_strcpy);
	vm->registerNativeFunction("phs__memory_write_native", StdLib::native_memory_write);
	vm->registerNativeFunction("phs__memory_write_offset_native", StdLib::native_memory_write_offset);
	vm->registerNativeFunction("phs__memory_write_string_native", StdLib::native_memory_write_string);
	vm->registerNativeFunction("phs__memory_write_string_offset_native", StdLib::native_memory_write_string_offset);
	vm->registerNativeFunction("phs__memory_read_native", StdLib::native_memory_read);
	vm->registerNativeFunction("phs__memory_read_offset_native", StdLib::native_memory_read_offset);
	vm->registerNativeFunction("phs__memory_read_string_native", StdLib::native_memory_read_string);
	vm->registerNativeFunction("phs__memory_read_string_offset_native", StdLib::native_memory_read_string_offset);
	vm->registerNativeFunction("phs__memory_free_native", StdLib::native_memory_free);
}

Value StdLib::var_free(const Value::ArrayInstance &args, VM *vm)
{
	checkArgCount(args, 1, "free");

	const Value &arg = args[0];

	if (!arg.isString())
		PHS_ERROR("free(): argument must be a string");

	vm->freeVariableByName(arg.string());
	return phsnull;
}

Value StdLib::native_memory_write(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 3, "write_native");

	if (!args[0].isInt())
		PHS_ERROR("Function 'write_native' expects an integer (pointer) as the first argument");
	i64 address = args[0].asInt();

	if (!args[1].isInt())
		PHS_ERROR("Function 'write_native' expects an integer (length) as the second argument");
	i64 length = args[1].asInt();

	if (!args[2].isInt())
		PHS_ERROR("Function 'write_native' expects an integer (data) as the third argument");
	i64 data = args[2].asInt();

	if (address == 0 || length == 0)
		PHS_ERROR("Neither length nor address can be empty!");

	if (std::cmp_greater(length, sizeof(i64)))
	{
		length = sizeof(i64);
	}

	auto* dst = static_cast<std::uint8_t*>(i64_to_pointer(address));

	std::memcpy(dst, &data, length);

	return phsnull;
}

Value StdLib::native_memory_write_string(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 3, "write_string_native");

	if (!args[0].isInt())
		PHS_ERROR("Function 'write_string_native' expects an integer (pointer) as the first argument");
	i64 address = args[0].asInt();

	if (!args[1].isInt())
		PHS_ERROR("Function 'write_string_native' expects an integer (length) as the second argument");
	i64 length = args[1].asInt();

	if (!args[2].isString())
		PHS_ERROR("Function 'write_string_native' expects a string (data) as the third argument");
	PhsString data = args[2].string();

	if (address == 0 || length == 0)
		PHS_ERROR("Neither length nor address can be empty!");

	auto* dst = static_cast<char*>(i64_to_pointer(address));

	size_t copyLength = std::min(
		static_cast<size_t>(length - 1),
		data.size()
	);

	std::memcpy(dst, data.c_str(), copyLength);

	dst[copyLength] = '\0';
	return phsnull;
}

Value StdLib::native_memory_write_offset(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 4, "write_offset_native");

	if (!args[0].isInt())
		PHS_ERROR("Function 'write_offset_native' expects an integer (pointer) as the first argument");
	i64 address = args[0].asInt();

	if (!args[1].isInt())
		PHS_ERROR("Function 'write_offset_native' expects an integer (offset) as the second argument");
	i64 offset = args[1].asInt();

	if (!args[2].isInt())
		PHS_ERROR("Function 'write_offset_native' expects an integer (length) as the third argument");
	i64 length = args[2].asInt();

	if (!args[3].isInt())
		PHS_ERROR("Function 'write_offset_native' expects an integer (data) as the fourth argument");
	i64 data = args[3].asInt();

	if (address == 0 || length == 0)
		PHS_ERROR("Neither length nor address can be empty!");

	if (std::cmp_greater(length ,sizeof(i64))) 
	{
		length = sizeof(i64);
	}

	auto* dst = static_cast<std::uint8_t*>(i64_to_pointer(address));
	dst += offset;

	std::memcpy(dst, &data, length);

	return phsnull;
}

Value StdLib::native_memory_write_string_offset(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 4, "write_string_offset_native");

	if (!args[0].isInt())
		PHS_ERROR("Function 'write_string_offset_native' expects an integer (pointer) as the first argument");
	i64 address = args[0].asInt();

	if (!args[1].isInt())
		PHS_ERROR("Function 'write_string_offset_native' expects an integer (offset) as the second argument");
	i64 offset = args[1].asInt();

	if (!args[2].isInt())
		PHS_ERROR("Function 'write_string_offset_native' expects an integer (length) as the third argument");
	i64 length = args[2].asInt();

	if (!args[3].isString())
		PHS_ERROR("Function 'write_string_offset_native' expects a string (data) as the fourth argument");
	PhsString data = args[3].string();

	if (address == 0 || length == 0)
		PHS_ERROR("Neither length nor address can be empty!");

	auto* dst = static_cast<char*>(i64_to_pointer(address));
	dst += offset;

	size_t copyLength = std::min(
		static_cast<size_t>(length - 1),
		data.size()
	);

	std::memcpy(dst, data.c_str(), copyLength);

	dst[copyLength] = '\0';

	return phsnull;
}

PhsString StdLib::native_memory_read_string(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "read_string_native");

	if (!args[0].isInt())
		PHS_ERROR("Function 'read_string_native' expects an integer (pointer) as the first argument");
	i64 address = args[0].asInt();

	if (!args[1].isInt())
		PHS_ERROR("Function 'read_string_native' expects an integer (length) as the second argument");
	i64 length = args[1].asInt();

	if (address == 0 || length == 0)
		PHS_ERROR("Neither length nor address can be empty!");

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

i64 StdLib::native_memory_read(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "read_native");

	if (!args[0].isInt())
		PHS_ERROR("Function 'read_native' expects an integer (pointer) as the first argument");
	i64 address = args[0].asInt();

	if (!args[1].isInt())
		PHS_ERROR("Function 'read_native' expects an integer (length) as the second argument");
	i64 length = args[1].asInt();

	if (address == 0 || length == 0)
		PHS_ERROR("Neither length nor address can be empty!");

	if (std::cmp_greater(length ,sizeof(i64)))
	{
		length = sizeof(i64);
	}

	auto* src = static_cast<std::uint8_t*>(i64_to_pointer(address));

	i64 data = 0;
	std::memcpy(&data, src, static_cast<std::size_t>(length));

	return data;
}

PhsString StdLib::native_memory_read_string_offset(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 3, "read_string_native_offset");

	if (!args[0].isInt())
		PHS_ERROR("Function 'read_string_native_offset' expects an integer (pointer) as the first argument");
	i64 address = args[0].asInt();

	if (!args[1].isInt())
		PHS_ERROR("Function 'read_string_native_offset' expects an integer (offset) as the second argument");
	i64 offset = args[1].asInt();

	if (!args[2].isInt())
		PHS_ERROR("Function 'read_string_native_offset' expects an integer (length) as the third argument");
	i64 length = args[2].asInt();

	if (address == 0 || length == 0)
		PHS_ERROR("Neither length nor address can be empty!");

	const auto* src = static_cast<const char*>(i64_to_pointer(address));
	src += offset;

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

i64 StdLib::native_memory_read_offset(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 3, "read_native_offset");

	if (!args[0].isInt())
		PHS_ERROR("Function 'read_native_offset' expects an integer (pointer) as the first argument");
	i64 address = args[0].asInt();

	if (!args[1].isInt())
		PHS_ERROR("Function 'read_native_offset' expects an integer (offset) as the second argument");
	i64 offset = args[1].asInt();

	if (!args[2].isInt())
		PHS_ERROR("Function 'read_native_offset' expects an integer (length) as the third argument");
	i64 length = args[2].asInt();

	if (address == 0 || length == 0)
		PHS_ERROR("Neither length nor address can be empty!");

	if (std::cmp_greater(length ,sizeof(i64)))
	{
		length = sizeof(i64);
	}

	auto* src = static_cast<std::uint8_t*>(i64_to_pointer(address));
	src += offset;

	i64 data = 0;
	std::memcpy(&data, src, static_cast<std::size_t>(length));

	return data;
}

i64 StdLib::native_memory_malloc(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "malloc_native");

	if (!args[0].isInt())
		PHS_ERROR("Function 'malloc_native' expects an integer (length) as the first argument");

	i64 length = args[0].asInt();

	if (length == 0)
		PHS_ERROR("length cannot be empty!");

	void* ptr = std::malloc(static_cast<std::size_t>(length));

	if (ptr == nullptr)
		PHS_ERROR("malloc failed");

	return pointer_to_i64(ptr);
}

i64 StdLib::native_memory_calloc(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "calloc_native");

	if (!args[0].isInt())
		PHS_ERROR("Function 'calloc_native' expects an integer (length) as the first argument");

	i64 length = args[0].asInt();

	if (length == 0)
		PHS_ERROR("length cannot be empty!");

	void* ptr = std::calloc(1, static_cast<std::size_t>(length));

	if (ptr == nullptr)
		PHS_ERROR("calloc failed");

	return pointer_to_i64(ptr);
}

i64 StdLib::native_memory_realloc(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "realloc_native");

	if (!args[0].isInt())
		PHS_ERROR("Function 'realloc_native' expects an integer (pointer) as the first argument");

	if (!args[1].isInt())
		PHS_ERROR("Function 'realloc_native' expects an integer (length) as the second argument");

	i64 ptrValue = args[0].asInt();
	i64 length = args[1].asInt();

	if (length == 0)
		PHS_ERROR("length cannot be empty!");

	void* oldPtr = i64_to_pointer(ptrValue);

	void* newPtr = std::realloc(oldPtr, static_cast<std::size_t>(length));

	if (newPtr == nullptr)
		PHS_ERROR("realloc failed");

	return pointer_to_i64(newPtr);
}

Value StdLib::native_memory_free(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "free_native");

	if (!args[0].isInt())
		PHS_ERROR("Function 'free_native' expects an integer (pointer) as the first argument");

	i64 address = args[0].asInt();

	if (address == 0)
		PHS_ERROR("Cannot free a null pointer");

	void* ptr = i64_to_pointer(address);

	std::free(ptr);

	return phsnull;
}

Value StdLib::native_memory_strcpy(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 4, "strcpy_native");

	if (!args[0].isInt())
		PHS_ERROR("Function 'strcpy_native' expects an integer (destination pointer) as the first argument");
	i64 dstAddress = args[0].asInt();

	if (!args[1].isInt())
		PHS_ERROR("Function 'strcpy_native' expects an integer (destination size) as the second argument");
	i64 dstSize = args[1].asInt();

	if (!args[2].isInt())
		PHS_ERROR("Function 'strcpy_native' expects an integer (source pointer) as the third argument");
	i64 srcAddress = args[2].asInt();

	if (!args[3].isInt())
		PHS_ERROR("Function 'strcpy_native' expects an integer (source size) as the fourth argument");
	i64 srcSize = args[3].asInt();

	if (dstAddress == 0 || srcAddress == 0)
		PHS_ERROR("Neither destination nor source pointer can be null!");

	if (dstSize <= 0 || srcSize <= 0)
		PHS_ERROR("Neither destination nor source size can be non-positive!");

	const auto* src = static_cast<const char*>(i64_to_pointer(srcAddress));
	auto* dst = static_cast<char*>(i64_to_pointer(dstAddress));

	size_t srcLen = 0;
	while (std::cmp_less(srcLen ,srcSize) && src[srcLen] != '\0')
	{
		srcLen++;
	}

	if (std::cmp_greater_equal(srcLen ,srcSize))
		PHS_ERROR("Source string is not null-terminated within the given source size");

	if (srcLen + 1 > static_cast<size_t>(dstSize))
		PHS_ERROR("Destination buffer is too small to hold the source string");

	std::memcpy(dst, src, srcLen + 1);

	return phsnull;
}

Value StdLib::native_memory_stralloc(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "stralloc_native");

	if (!args[0].isString())
		PHS_ERROR("Function 'stralloc_native' expects a string as the first argument");

	PhsString data = args[0].string();
	size_t bufSize = data.size() + 1;

	void* ptr = std::malloc(bufSize);
	if (ptr == nullptr)
		PHS_ERROR("malloc failed");

	std::memcpy(ptr, data.c_str(), bufSize);

	i64 address = pointer_to_i64(ptr);
	return Value::createArray({address, static_cast<i64>(bufSize)});
}

i64 StdLib::native_memory_argv(const Value::ArrayInstance &args, VM *) {
	checkArgCount(args, 0, "phs__argv_ptr");
	return pointer_to_i64(&argv);
}

i64 StdLib::native_memory_argc(const Value::ArrayInstance &args, VM *){
	checkArgCount(args, 0, "phs__argc_ptr");
	return pointer_to_i64(&argc);
}

} // namespace Phasor