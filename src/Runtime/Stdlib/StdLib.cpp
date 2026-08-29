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

#include "StdLib.hpp"
#include <cassert>
#include <cstdlib>
#include <platform.h>
#include <mutex>
#include <thread>
#include <chrono>
#include <utility>

#if !defined(_WIN32)
#include <sys/wait.h>
#endif

namespace Phasor
{

namespace {
void processReaperLoop()
{
	using namespace std::chrono_literals;
	for (;;)
	{
		std::this_thread::sleep_for(100ms);
		std::lock_guard<std::mutex> lock(Phasor::StdLib::getProcessPoolMutex());
		for (auto &entry : Phasor::StdLib::getProcessPool())
		{
			if (!entry.handle) continue;
			Phasor::StdLib::pollProcessExitLocked(*entry.handle);
			if (entry.handle->exited && entry.handle->forgotten)
			{
#if defined(_WIN32)
				if (entry.handle->nativeHandle) CloseHandle(static_cast<HANDLE>(entry.handle->nativeHandle));
#endif
				entry.handle.reset();
			}
		}
	}
}
} // namespace

void StdLib::ensureReaperStarted()
{
	static std::once_flag once;
	std::call_once(once, [] { std::thread(processReaperLoop).detach(); });
}

void StdLib::registerInternalFunctions(VM *vm)
{
	vm->registerNativeFunction("phs__is_32", [](const Value::ArrayInstance &, VM *)
	{
#if defined(PHS_IS_32)
		return true
#else
		return false;
#endif		
	});
	vm->registerNativeFunction("phs__trace", [](const Value::ArrayInstance &, VM *)
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
	vm->registerNativeFunction("to_int", StdLib::to_int);
	vm->registerNativeFunction("printf", StdLib::io_printf);
	vm->registerNativeFunction("c_fmt", StdLib::io_c_format);
	vm->registerNativeFunction("phs_push", StdLib::meta_push);
	vm->registerNativeFunction("arr_push", StdLib::array_push);
	vm->registerNativeFunction("phs_pop", StdLib::meta_pop);
	vm->registerNativeFunction("get_type", StdLib::get_type);
	vm->registerNativeFunction("error", StdLib::sys_error);
	vm->registerNativeFunction("sys_os", StdLib::sys_os);
	vm->registerNativeFunction("sys_arch", StdLib::sys_arch);
	vm->registerNativeFunction("phs_version", StdLib::meta_get_version);
}

std::mutex& StdLib::getProcessPoolMutex()
{
	static std::mutex m;
	return m;
}

std::vector<StdLib::ProcessEntry>& StdLib::getProcessPool()
{
	static std::vector<StdLib::ProcessEntry> pool;
	return pool;
}

i64 StdLib::allocProcessHandle(std::unique_ptr<ProcessHandle> h)
{
	std::lock_guard<std::mutex> lock(getProcessPoolMutex());
	auto& pool = getProcessPool();
	for (size_t i = 0; i < pool.size(); ++i)
		if (!pool[i].handle) { pool[i].handle = std::move(h); ensureReaperStarted(); return static_cast<i64>(i); }
	pool.push_back({std::move(h)});
	ensureReaperStarted();
	return static_cast<i64>(pool.size() - 1);
}

StdLib::ProcessHandle* StdLib::getProcessHandleLocked(i64 h)
{
	auto& pool = getProcessPool();
	if (h >= 0 && std::cmp_less(h, pool.size()) && pool[h].handle)
		return pool[h].handle.get();
	return nullptr;
}

bool StdLib::pollProcessExitLocked(ProcessHandle &proc)
{
	if (proc.exited) return true;
#if defined(_WIN32)
	if (WaitForSingleObject(static_cast<HANDLE>(proc.nativeHandle), 0) != WAIT_OBJECT_0)
		return false;
	DWORD code = 0;
	GetExitCodeProcess(static_cast<HANDLE>(proc.nativeHandle), &code);
	proc.exitCode = static_cast<int>(code);
#else
	int status = 0;
	pid_t r = waitpid(static_cast<pid_t>(proc.pid), &status, WNOHANG);
	if (r <= 0) return false;
	proc.exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif
	proc.exited = true;
	return true;
}

void StdLib::releaseProcessHandleLocked(i64 h)
{
	auto& pool = getProcessPool();
	if (h < 0 || std::cmp_greater_equal(h, pool.size()) || !pool[h].handle) return;
#if defined(_WIN32)
	if (pool[h].handle->nativeHandle) CloseHandle(static_cast<HANDLE>(pool[h].handle->nativeHandle));
#endif
	pool[h].handle.reset();
}

std::vector<StdLib::FileHandle>& StdLib::getFilePool()
{
	static std::vector<FileHandle> pool;
	return pool;
}

std::iostream* StdLib::getFileDescriptor(i64 fd)
{
	auto& pool = StdLib::getFilePool();
	if (fd >= 0 && std::cmp_less(fd, pool.size()) && pool[fd].stream)
		return pool[fd].stream.get();
	return nullptr;
}

i64 StdLib::allocFileDescriptor(std::unique_ptr<std::iostream> stream, StreamKind kind, i64 ownerProcess)
{
	auto& pool = getFilePool();
	i64 fd = -1;
	for (size_t i = 0; i < pool.size(); ++i)
	{
		if (!pool[i].stream)
		{
			pool[i].stream = std::move(stream);
			pool[i].kind = kind;
			pool[i].ownerProcess = ownerProcess;
			fd = static_cast<i64>(i);
			break;
		}
	}
	if (fd < 0)
	{
		pool.push_back({std::move(stream), kind, ownerProcess});
		fd = static_cast<i64>(pool.size() - 1);
	}
	return fd;
}

void StdLib::requireString(const Value &v, const char *fnName, const char *what)
{
	if (!v.isString())
		PHS_ERROR(std::string(fnName) + "() expects a string as its " + what);
}

void StdLib::requireInt(const Value &v, const char *fnName, const char *what)
{
	if (!v.isInt())
		PHS_ERROR(std::string(fnName) + "() expects an integer as its " + what);
}

void StdLib::requireBool(const Value &v, const char *fnName, const char *what)
{
	if (!v.isBool())
		PHS_ERROR(std::string(fnName) + "() expects a boolean as its " + what);
}

void StdLib::requireNumber(const Value &v, const char *fnName, const char *what)
{
	if (!v.isNumber())
		PHS_ERROR(std::string(fnName) + "() expects a number as its " + what);
}

void StdLib::requireArray(const Value &v, const char *fnName, const char *what)
{
	if (!v.isArray())
		PHS_ERROR(std::string(fnName) + "() expects an array as its " + what);
}

void StdLib::requireStruct(const Value &v, const char *fnName, const char *what)
{
	if (!v.isStruct())
		PHS_ERROR(std::string(fnName) + "() expects a struct as its " + what);
}

Phasor::i64 StdLib::requireAddress(const Value &v, const char *fnName, const char *what)
{
	requireInt(v, fnName, what);
	i64 address = v.asInt();
	if (address == 0)
		PHS_ERROR(std::string(fnName) + "(): " + what + " cannot be a null pointer");
	return address;
}

Phasor::i64 StdLib::requireLength(const Value &v, const char *fnName, const char *what)
{
	requireInt(v, fnName, what);
	i64 length = v.asInt();
	if (length <= 0)
		PHS_ERROR(std::string(fnName) + "(): " + what + " must be greater than zero");
	return length;
}

std::unordered_map<Phasor::string, std::function<void(Phasor::VM *)>> StdLib::modules{
	    {"io", registerIOFunctions},
		{"data", registerDataFunctions},
	    {"sys", registerSysFunctions},
	    {"math", registerMathFunctions},
	    {"str", registerStringFunctions},
	    {"type", registerTypeConvFunctions},
	    {"meta", registerMetaFunctions},
	    {"mem", registerMemoryFunctions},
	    {"rand", registerRandomFunctions},
		{"array", registerArrayFunctions},
		{"struct", registerObjectFunctions},
		{"ini", registerIniFunctions},
#ifndef SANDBOXED
		{"net", registerNetFunctions},
		{"http", registerHttpFunctions},
	    {"file", registerFileFunctions},
#endif
	    {"*",
	     [](Phasor::VM *vm)
		 {
		     registerIOFunctions(vm);
			 registerDataFunctions(vm);
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
			 registerNetFunctions(vm);
			 registerHttpFunctions(vm);
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

std::vector<Phasor::string> StdLib::phasorStringArrayToVector(const Phasor::Value &arr)
{
	if (!arr.isArray())
		PHS_ERROR("phasorStringArrayToVector() expects an array value");

	auto elements = arr.asArray();

	std::vector<Phasor::string> result;
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


void StdLib::checkArgCount(const Value::ArrayInstance &args, size_t minimumArguments, const std::string &name,
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

bool StdLib::std_import(const Value::ArrayInstance &args, VM *vm)
{
	checkArgCount(args, 1, "phs__std", true);

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
Value StdLib::std_assert(const Value::ArrayInstance &args, VM *vm)
#else
Value StdLib::std_assert(const Value::ArrayInstance &args, VM *)
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
