#include "StdLib.hpp"
#include <atomic>
#include <chrono>
#include <thread>
#include <print>
#include <userconsent.h>
#if defined(_MSC_VER)
#include <vcruntime_startup.h>
#endif
#include <phsint.hpp>

#include "core/system.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

bool consentAskedCLI{false};
bool consentGrantedCLI{false};

bool consentAskedEnv{false};
bool consentGrantedEnv{false};

namespace Phasor
{

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
#endif

void StdLib::registerSysFunctions(VM *vm)
{
#ifndef SANDBOXED
	vm->registerNativeFunction("sys_os", StdLib::sys_os);
	vm->registerNativeFunction("sys_get_memory", StdLib::sys_get_free_memory);
	vm->registerNativeFunction("wait_for_input", StdLib::sys_wait_for_input);
	vm->registerNativeFunction("sys_shell", StdLib::sys_shell);
	vm->registerNativeFunction("sys_fork", StdLib::sys_fork);
	vm->registerNativeFunction("sys_fork_detached", StdLib::sys_fork_detached);
	vm->registerNativeFunction("error", StdLib::sys_crash);
	vm->registerNativeFunction("reset", StdLib::sys_reset);
	vm->registerNativeFunction("sys_pid", StdLib::sys_pid);
	vm->registerNativeFunction("isatty", StdLib::sys_isatty);
	vm->registerNativeFunction("sys_env", StdLib::sys_env);
	vm->registerNativeFunction("sys_args", StdLib::sys_args);
	vm->registerNativeFunction("sys_argc", StdLib::sys_argc);
	vm->registerNativeFunction("sys_argv", StdLib::sys_argv);
#else
	auto stub = [](const std::vector<Value> &, VM *) -> Value { return phsnull };
	vm->registerNativeFunction("sys_os", [](const std::vector<Value> &, VM *) { return "sandbox"; });
	vm->registerNativeFunction("sys_get_memory", stub);
	vm->registerNativeFunction("sys_pid", stub);
	vm->registerNativeFunction("isatty", stub);
	if (!std::getenv("PHASOR_NO_ENV")) {
		vm->registerNativeFunction("sys_env", [] (const std::vector<Value> &v, VM *vm) -> Value {
			if (consentGrantedEnv) {
				return sys_env(v, vm);
			}
			if (!consentAskedEnv) {
				[[unlikely]]
				consentGrantedEnv = prompt_consent("Standard library", EConsentVolition::Wants, "use", "environment variables"); 
				consentAskedEnv = true;
			}
			return phsnull;
		});
		vm->registerNativeFunction("sys_args", [] (const std::vector<Value> &v, VM *vm) {
			if (consentGrantedCLI) {
				return sys_argc(v, vm);
			}
			if (!consentAskedCLI) {
				[[unlikely]]
				consentGrantedCLI = prompt_consent("Standard library", EConsentVolition::Wants, "use", "command line arguments"); 
				consentAskedCLI = true;
			}
			return phsnull;
		});
	}
#endif
	vm->registerNativeFunction("time", StdLib::sys_time);
	vm->registerNativeFunction("timef", StdLib::sys_time_formatted);
	vm->registerNativeFunction("sleep", StdLib::sys_sleep);
	vm->registerNativeFunction("shutdown", StdLib::sys_shutdown);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

f64 StdLib::sys_time(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "time");
	auto   now = std::chrono::steady_clock::now();
	auto   duration = now.time_since_epoch();
	f64 millis = std::chrono::duration<f64, std::milli>(duration).count();
	return millis;
}

Value StdLib::sys_time_formatted(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "timef");
	PhsString format = args[0].asString();

	auto        now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);

	std::tm tm{};
#if defined(_WIN32)
	localtime_s(&tm, &t);
#else
	localtime_r(&t, &tm);
#endif

	char buffer[256];
	if (std::strftime(buffer, sizeof(buffer), format.c_str(), &tm) == 0)
	{
		return Value(" ");
	}

	return PhsString(buffer);
}

Value StdLib::sys_sleep(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "sleep");
	i64 ms = args[0].asInt();
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	return Value(" ");
}

Value StdLib::sys_env(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "sys_env");
	PhsString key = args[0].asString();
	PhsString value;
	dupenv_ret result = dupenv(value, key.c_str());
	if (result == dupenv_ret::NotFound) return false;
	else if (result == dupenv_ret::Success) return value;
	else return phsnull;
}

i64 StdLib::sys_argc(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "sys_args");
	return static_cast<i64>(argc);
}

Value StdLib::sys_argv(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "sys_argv");
	i64 index = args[0].asInt();
	if (index < 0 || index >= argc) return phsnull;
	return argv[index];
}

Value StdLib::sys_args(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "sys_args");
	std::vector<Value> arguments;
	for (int i = 0; i < argc; ++i)
	{
		arguments.push_back(Value(argv[i]));
	}
	return Value::createArray(std::move(arguments));
}

Value StdLib::sys_shutdown(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "shutdown");
	int ret = static_cast<int>(args[0].asInt());
	vm->setStatus(ret);
	throw VM::Halt();
}

#ifndef SANDBOXED

PhsString StdLib::sys_os(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "sys_os");
#if defined(_WIN32)
	return "win32";
#elif defined(__linux__)
	return "Linux";
#elif defined(__APPLE__)
	return "Darwin";
#elif defined(__FreeBSD__)
	return "FreeBSD";
#elif defined(__unix__)
	return "UNIX";
#else
	return "Unknown";
#endif
}

i64 StdLib::sys_get_free_memory(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "sys_get_memory");
	return static_cast<i64>(PHASORstd_sys_getAvailableMemory());
}

Value StdLib::sys_wait_for_input(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "wait_for_input");
	io_gets({}, vm);
	return Value("");
}

Value StdLib::sys_shell(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "sys_shell");
	return vm->regRun(OpCode::SYSTEM_R, args[0]);
}

i64 StdLib::sys_fork(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "sys_fork", true);
	const char         *executable = args[0].c_str();
	int                 argc = (int)args.size() - 1;
	std::vector<char *> v_argv(argc);
	for (int i = 0; i < argc; ++i)
	{
		v_argv[i] = const_cast<char *>(args[i + 1].c_str());
	}
	return static_cast<i64>(PHASORstd_sys_run(executable, argc, v_argv.data()));
}

i64 StdLib::sys_fork_detached(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "sys_fork_detached", true);
	const char         *executable = args[0].c_str();
	int                 argc = (int)args.size() - 1;
	std::vector<char *> v_argv(argc);
	for (int i = 0; i < argc; ++i)
	{
		v_argv[i] = const_cast<char *>(args[i + 1].c_str());
	}
	return static_cast<i64>(PHASORstd_sys_run_detached(executable, argc, v_argv.data()));
}

Value StdLib::sys_crash(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "error", true);
	vm->reset();
	vm->setStatus(-1);
	throw std::runtime_error(args[0].asString());
}

Value StdLib::sys_reset(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "reset");
	vm->reset();
	return phsnull;
}

i64 StdLib::sys_pid(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "sys_pid");
#if defined(_WIN32)
	return static_cast<i64>(GetCurrentProcessId());
#else
	return static_cast<i64>(getpid());
#endif
}

Value StdLib::sys_isatty(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "isatty");
#ifdef _WIN32
	return _isatty(_fileno(stdin));
#else
	return isatty(fileno(stdin));
#endif
}

#endif
} // namespace Phasor
