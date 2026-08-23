#include "StdLib.hpp"
#include <cassert>
#include <cstdlib>
#include <platform.h>

namespace Phasor
{
void StdLib::registerInternalFunctions(VM *vm)
{
	vm->registerNativeFunction("phs__is_32", [](const std::vector<Value> &, VM *)
	{
#if defined(PHS_IS_32)
		return true
#else
		return false;
#endif		
	});
	vm->registerNativeFunction("phs__trace", [](const std::vector<Value> &, VM *)
	{
#if defined(TRACING)
		return true;
#else
		return false;
#endif
	});
	vm->registerNativeFunction("rand_crypto_int", StdLib::rand_get_crypto_int);
	vm->registerNativeFunction("phs__argc_ptr", StdLib::native_memory_argc);
	vm->registerNativeFunction("phs__argv_ptr", StdLib::native_memory_argv);
	vm->registerNativeFunction("phs__memory_malloc_native", StdLib::native_memory_malloc);
	vm->registerNativeFunction("phs__memory_write_native", StdLib::native_memory_write);
	vm->registerNativeFunction("phs__memory_write_offset_native", StdLib::native_memory_write_offset);
	vm->registerNativeFunction("phs__memory_read_native", StdLib::native_memory_read);
	vm->registerNativeFunction("phs__memory_read_offset_native", StdLib::native_memory_read_offset);
	vm->registerNativeFunction("phs__memory_read_string_native", StdLib::native_memory_read_string);
	vm->registerNativeFunction("phs__memory_free_native", StdLib::native_memory_free);
	vm->registerNativeFunction("shutdown", StdLib::sys_shutdown);
	vm->registerNativeFunction("printf", StdLib::io_printf);
	vm->registerNativeFunction("c_fmt", StdLib::io_c_format);
	vm->registerNativeFunction("phs_push", StdLib::meta_push);
	vm->registerNativeFunction("arr_push", StdLib::array_push);
	vm->registerNativeFunction("phs_pop", StdLib::meta_pop);
	vm->registerNativeFunction("get_type", StdLib::get_type);
	vm->registerNativeFunction("error", StdLib::sys_crash);
	vm->registerNativeFunction("sys_os", StdLib::sys_os);
	vm->registerNativeFunction("sys_arch", StdLib::sys_arch);
	vm->registerNativeFunction("phs_version", StdLib::meta_get_version);
}

std::unordered_map<PhsString, std::function<void(Phasor::VM *)>> StdLib::modules{
	    {"stdio", registerIOFunctions},
	    {"stdsys", registerSysFunctions},
	    {"stdmath", registerMathFunctions},
	    {"stdstr", registerStringFunctions},
	    {"stdtype", registerTypeConvFunctions},
	    {"stdmeta", registerMetaFunctions},
	    {"stdmem", registerMemoryFunctions},
	    {"stdrand", registerRandomFunctions},
		{"stdarray", registerArrayFunctions},
		{"stdstruct", registerObjectFunctions},
		{"stdini", registerIniFunctions},
#ifndef SANDBOXED
	    {"stdfile", registerFileFunctions},
#endif
	    {"std*",
	     [](Phasor::VM *vm)
		 {
		     registerIOFunctions(vm);
		     registerSysFunctions(vm);
		     registerMathFunctions(vm);
		     registerStringFunctions(vm);
		     registerTypeConvFunctions(vm);
		     registerMetaFunctions(vm);
		     registerMemoryFunctions(vm);
		     registerRandomFunctions(vm);
			 registerArrayFunctions(vm);
			 registerObjectFunctions(vm);
#ifndef SANDBOXED
		     registerFileFunctions(vm);
#endif
	     }},
		 {"__phs_init", registerInternalFunctions},
	};

char **StdLib::argv = nullptr;
int    StdLib::argc = 0;

Phasor::i64 StdLib::pointer_to_i64(void* ptr)
{
	if (ptr == nullptr)
	{
		return 0;
	}

	static_assert(sizeof(void*) <= sizeof(Phasor::i64));

	Phasor::i64 value;
	std::memcpy(&value, &ptr, sizeof(ptr));

	return value;
}

void* StdLib::i64_to_pointer(Phasor::i64 value)
{
	if (value == 0)
	{
		return nullptr;
	}
	void* ptr;
	std::memcpy(&ptr, &value, sizeof(ptr));

	return ptr;
}

std::vector<Phasor::PhsString> StdLib::phasorStringArrayToVector(const Phasor::Value &arr)
{
	if (!arr.isArray())
		PHS_ERROR("phasorStringArrayToVector() expects an array value");

	auto elements = arr.asArray();

	std::vector<Phasor::PhsString> result;
	result.reserve(elements->size());

	for (const auto &elem : *elements)
	{
		if (!elem.isString())
			PHS_ERROR("phasorStringArrayToVector() expects an array of strings");

		result.push_back(elem.string());
	}

	return result;
}

std::vector<char *> StdLib::phasorStringArrayToCharArray(const Phasor::Value &arr, bool nullTerminate)
{
	if (!arr.isArray())
		PHS_ERROR("phasorStringArrayToCharArray() expects an array value");

	auto elements = arr.asArray();

	std::vector<char *> result;
	result.reserve(elements->size() + (nullTerminate ? 1 : 0));

	for (const auto &elem : *elements)
	{
		if (!elem.isString())
			PHS_ERROR("phasorStringArrayToCharArray() expects an array of strings");

		result.push_back(const_cast<char *>(elem.c_str()));
	}

	if (nullTerminate)
		result.push_back(nullptr);

	return result;
}


void StdLib::checkArgCount(const std::vector<Value> &args, size_t minimumArguments, const std::string &name,
                           bool allowMoreArguments)
{
	if (args.size() < minimumArguments)
	{
		PHS_ERROR("Function '" + name + "' expects at least " + std::to_string(minimumArguments) +
		                         " arguments, but got " + std::to_string(args.size()));
	}
	if (!allowMoreArguments && args.size() > minimumArguments)
	{
		PHS_ERROR("Function '" + name + "' expects exactly " + std::to_string(minimumArguments) +
		                         " arguments, but got " + std::to_string(args.size()));
	}
}

bool StdLib::std_import(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "ffiload", true);

	for (const auto &arg : args)
	{
		auto it = modules.find(arg.string());
		if (it != modules.end())
		{
			it->second(vm);
		} else {
			PHS_ERROR("Unknown module: " + arg.string());
		}
	}
	return true;
}

#ifndef SANDBOXED
#if defined(_DEBUG) || defined(TRACING)
Value StdLib::std_assert(const std::vector<Value> &args, VM *vm)
#else
Value StdLib::std_assert(const std::vector<Value> &args, VM *)
#endif
{
	checkArgCount(args, 1, "assert", true);

	if (args.size() > 2)
	{ [[unlikely]]
		PHS_ERROR("Assert expects 1 or 2 arguments, but got " + std::to_string(args.size()));
	}

#ifdef _DEBUG
	bool haveMessage = false;
	const char* message = nullptr;

	if (args.size() == 2)
	{
		message = args[1].c_str();
	}
#endif

#ifdef TRACING
#ifdef _DEBUG
	vm->log(std::format("({})({:T})\n", PHS_SRC_LOC(), args[0]));
#else
	vm->log(std::format("({})({:T}): Assertion skipped (NDEBUG)\n", PHS_SRC_LOC(), args[0]));
#endif
	vm->flush();
#endif

#ifdef _DEBUG
	if (!args[0].isTruthy())
	{ [[unlikely]]
		vm->logerr(std::format("({})({:T}): Assertion failed!\n", PHS_SRC_LOC(), args[0]));
		if (haveMessage) vm->logerr(std::format("{}\n", message));
		vm->flusherr();
	}
	assert(args[0].isTruthy());
#endif
	return phsnull;
}
#endif

} // namespace Phasor