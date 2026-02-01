#include "StdLib.hpp"
#include <atomic>
#include <chrono>
#include <thread>
#if defined(_MSC_VER)
#include <vcruntime_startup.h>
#endif

#include "core/system.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace Phasor
{

Value StdLib::registerSysFunctions(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "include_stdsys");
	vm->registerNativeFunction("time", StdLib::sys_time);
	vm->registerNativeFunction("timef", StdLib::sys_time_formatted);
	vm->registerNativeFunction("sleep", StdLib::sys_sleep);
	vm->registerNativeFunction("clear", StdLib::sys_clear);
	vm->registerNativeFunction("sys_os", StdLib::sys_os);
	vm->registerNativeFunction("sys_env", StdLib::sys_env);
	vm->registerNativeFunction("sys_argv", StdLib::sys_argv);
	vm->registerNativeFunction("sys_argc", StdLib::sys_argc);
	vm->registerNativeFunction("sys_get_memory", StdLib::system_get_free_memory);
	vm->registerNativeFunction("wait_for_input", StdLib::sys_wait_for_input);
	vm->registerNativeFunction("sys_execute", StdLib::sys_exec);
	vm->registerNativeFunction("error", StdLib::sys_crash);
	vm->registerNativeFunction("reset", StdLib::sys_reset);
	vm->registerNativeFunction("shutdown", StdLib::sys_shutdown);
	vm->registerNativeFunction("sys_pid", StdLib::sys_pid);
	vm->registerNativeFunction("sys_exec_output", StdLib::sys_exec_get_output);
	vm->registerNativeFunction("sys_exec_error", StdLib::sys_exec_get_error);
	return true;
}

Value StdLib::sys_time(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "time");
	auto   now = std::chrono::steady_clock::now();
	auto   duration = now.time_since_epoch();
	double millis = std::chrono::duration<double, std::milli>(duration).count();
	return millis;
}

Value StdLib::sys_time_formatted(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "timef");
	std::string format = args[0].asString();

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

	return std::string(buffer);
}

Value StdLib::sys_sleep(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "sleep");
	int64_t ms = args[0].asInt();
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	return Value(" ");
}

Value StdLib::sys_clear(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "clear");
	return io_prints(std::vector<Value>{"\033[2J\033[H"}, vm);
}

Value StdLib::sys_os(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "sys_os");
#if defined(_WIN32)
	return Value("win32");
#elif defined(__linux__)
	return Value("Linux");
#elif defined(__APPLE__)
	return Value("Darwin");
#elif defined(__FreeBSD__)
	return Value("FreeBSD");
#elif defined(__unix__)
	return Value("UNIX");
#else
	return Value("Unknown");
#endif
	return false;
}

Value StdLib::sys_env(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "sys_env");
	std::string key = args[0].asString();
	std::string value;
	dupenv(value, key.c_str(), envp);
	return value;
}

Value StdLib::sys_argv(const std::vector<Value> &args, VM *)
{
	if (args.size() == 0)
	{
		auto l_argv = std::make_shared<Value::StructInstance>();

		for (size_t i = 0; i < static_cast<size_t>(argc); i++)
		{
			l_argv->fields["arg" + std::to_string(i)] = Value(argv[i]);
		}

		return Value(l_argv);
	}

	checkArgCount(args, 1, "sys_argv");
	int64_t index = args[0].asInt();
	if (argv != nullptr)
		return argv[index];
	else
		return Value();
}

Value StdLib::sys_argc(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "sys_argc");
	return argc;
}

Value StdLib::system_get_free_memory(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "sys_get_memory");
	return static_cast<int64_t>(getAvailableMemory());
}

Value StdLib::sys_wait_for_input(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "wait_for_input");
	io_gets({}, vm);
	return Value("");
}

Value StdLib::sys_exec(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "sys_execute");
	vm->setRegister(VM::Register::r1, args[0]); // Load string into r1
	return vm->operation(OpCode::SYSTEM_R, VM::Register::r1);
}

Value StdLib::sys_exec_get_output(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "sys_exec_output");
	vm->setRegister(VM::Register::r1, args[0]); // Load string into r1
	return vm->operation(OpCode::SYSTEM_OUT_R, VM::Register::r1);
}

Value StdLib::sys_exec_get_error(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "sys_exec_error");
	vm->setRegister(VM::Register::r1, args[0]); // Load string into r1
	return vm->operation(OpCode::SYSTEM_ERR_R, VM::Register::r1);
}

Value StdLib::sys_crash(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "error");
	throw std::runtime_error(args[0].asString());
	vm->reset();
}

Value StdLib::sys_reset(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "reset");
	vm->reset();
	return Value();
}

Value StdLib::sys_shutdown(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "shutdown");
	vm->reset();
	int ret = static_cast<int>(args[0].asInt());
	std::exit(ret);
	return Value(ret);
}

Value StdLib::sys_pid(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "sys_pid");
#if defined(_WIN32)
	return static_cast<int64_t>(GetCurrentProcessId());
#else
	return static_cast<int64_t>(getpid());
#endif
}

} // namespace Phasor
