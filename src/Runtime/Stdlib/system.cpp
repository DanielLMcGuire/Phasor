#include "StdLib.hpp"
#include <atomic>
#include <chrono>
#include <thread>
#include <vcruntime_startup.h>

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
	vm->registerNativeFunction("wait_for_input", StdLib::sys_wait_for_input);
	vm->registerNativeFunction("sys_execute", StdLib::sys_exec);
	vm->registerNativeFunction("error", StdLib::sys_crash);
	vm->registerNativeFunction("reset", StdLib::sys_reset);
	vm->registerNativeFunction("shutdown", StdLib::sys_shutdown);
	return true;
}

Value StdLib::sys_time(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "time");
	auto now = std::chrono::system_clock::now();
	auto duration = now.time_since_epoch();
	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	return static_cast<int64_t>(millis);
}

Value StdLib::sys_time_formatted(const std::vector<Value> &args, VM *vm)
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

Value StdLib::sys_sleep(const std::vector<Value> &args, VM *vm)
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

Value StdLib::sys_os(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "sys_os");
#if defined(_WIN32)
	return Value("NT");
#elif defined(__linux__)
	return Value("Linux");
#elif defined(__APPLE__)
	return Value("Darwin");
#elif defined(__FreeBSD__)
	return Value("FreeBSD");
#else
	return Value("Unknown");
#endif
	return false;
}

Value StdLib::sys_env(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "sys_env");
	std::string key = args[0].asString();
	std::string value;
	dupenv(value, key.c_str(), envp);
	return value;
}

Value StdLib::sys_argv(const std::vector<Value> &args, VM *vm)
{
	if (args.size() == 0) 
	{
		auto l_argv = std::make_shared<Value::StructInstance>();
		
		for (size_t i = 0; i < argc; i++)
		{
			l_argv->fields["arg" + std::to_string(i)] = Value(argv[i]);
		}
		
		return Value(l_argv);
	}

	checkArgCount(args, 1, "sys_argv");
	int64_t index = args[0].asInt();
	return argv[index];
}

Value StdLib::sys_argc(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "sys_argc");
	return argc;
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

Value StdLib::sys_crash(const std::vector <Value> &args, VM *vm)
{
	checkArgCount(args, 1, "error");
	throw std::runtime_error(args[0].asString());
	vm->reset();
}

Value StdLib::sys_reset(const std::vector <Value> &args, VM *vm)
{
	checkArgCount(args, 0, "reset");
	vm->reset();
	// NOTE: Will cause crash if not used properly!
	return Value();
}

Value StdLib::sys_shutdown(const std::vector <Value> &args, VM *vm)
{
	checkArgCount(args, 1, "shutdown");
	vm->reset();
	int ret = args[0].asInt();
	std::exit(ret);
	return Value(ret);
}