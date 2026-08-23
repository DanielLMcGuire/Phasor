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
#include <ctime>
#include <iomanip>
#include <sstream>
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
	vm->registerNativeFunction("sys_os", StdLib::sys_arch);
	vm->registerNativeFunction("sys_get_memory", StdLib::sys_get_free_memory);
	vm->registerNativeFunction("wait_for_input", StdLib::sys_wait_for_input);
	vm->registerNativeFunction("sys_shell", StdLib::sys_shell);
	vm->registerNativeFunction("sys_fork", StdLib::sys_fork);
	vm->registerNativeFunction("sys_fork_output", StdLib::sys_fork_output);
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
	auto stub = [](const std::vector<Value> &, VM *) -> Value { return phsnull; };
	vm->registerNativeFunction("sys_os", [](const std::vector<Value> &, VM *) { return 6; });
	vm->registerNativeFunction("sys_arch", [](const std::vector<Value> &, VM *) { return 5; });
	vm->registerNativeFunction("sys_get_memory", stub);
	vm->registerNativeFunction("sys_pid", stub);
	vm->registerNativeFunction("isatty", stub);
	if (!std::getenv("PHASOR_NO_ENV"))
	{
		vm->registerNativeFunction("sys_env", [] (const std::vector<Value> &v, VM *vm) -> Value {
			if (consentGrantedEnv)
			{
				return sys_env(v, vm);
			}
			if (!consentAskedEnv)
			{
				[[unlikely]]
				consentGrantedEnv = prompt_consent("Standard library", EConsentVolition::Wants, "use", "environment variables"); 
				consentAskedEnv = true;
			}
			return phsnull;
		});
		vm->registerNativeFunction("sys_args", [] (const std::vector<Value> &v, VM *vm)
		{
			if (consentGrantedCLI)
			{
				return sys_args(v, vm);
			}
			if (!consentAskedCLI)
			{
				[[unlikely]]
				consentGrantedCLI = prompt_consent("Standard library", EConsentVolition::Wants, "use", "command line arguments"); 
				consentAskedCLI = true;
			}
			return phsnull;
		});
	}
#endif
	vm->registerNativeFunction("time", StdLib::sys_time);
	vm->registerNativeFunction("localtime", StdLib::sys_time_local);
	vm->registerNativeFunction("timef", StdLib::sys_time_formatted);
	vm->registerNativeFunction("localtimef", StdLib::sys_time_formatted_local);
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
    if (!args[0].isString())
        PHS_ERROR("timef() expects a string as its argument (format)");
    std::string format = args[0].string();

    auto now = std::chrono::system_clock::now();

    try {
        return PhsString(std::vformat("{:" + format + "}", std::make_format_args(now)));
    } catch (const std::format_error &) {
        return {" "};
    }
}

f64 StdLib::sys_time_local(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 0, "time");
    auto now = std::chrono::system_clock::now();

#if defined(__APPLE__) || defined(__ANDROID__) || (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION < 190000)
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm;
    localtime_r(&t, &local_tm);

    f64 utc_millis = std::chrono::duration<f64, std::milli>(now.time_since_epoch()).count();
    f64 offset_millis = local_tm.tm_gmtoff * 1000.0;

    return utc_millis + offset_millis;
#else
    auto zoned = std::chrono::zoned_time{std::chrono::current_zone(), now};
    auto local = zoned.get_local_time();
    return std::chrono::duration<f64, std::milli>(local.time_since_epoch()).count();
#endif
}

Value StdLib::sys_time_formatted_local(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 1, "localtimef");
    if (!args[0].isString())
        PHS_ERROR("localtimef() expects a string as its argument (format)");
    std::string format = args[0].string();

    auto now = std::chrono::system_clock::now();

#if defined(__APPLE__) || defined(__ANDROID__) || (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION < 190000)
    std::time_t time_now = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm;
    localtime_r(&time_now, &local_tm);

    std::ostringstream oss;
    oss << std::put_time(&local_tm, format.c_str());
    
    return PhsString(oss.str());
#else
    auto zoned = std::chrono::zoned_time{std::chrono::current_zone(), now};
    return PhsString(std::vformat("{:" + format + "}", std::make_format_args(zoned)));
#endif
}

Value StdLib::sys_sleep(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "sleep");
	if (!args[0].isNumber())
		PHS_ERROR("sleep() expects a number as its argument (milliseconds)");
	double ms = args[0].asFloat();
	std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(ms));
	return {" "};
}

Value StdLib::sys_env(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "sys_env");
	if (!args[0].isString())
		PHS_ERROR("sys_env() expects a string as its argument (key)");
	PhsString key = args[0].string();
	PhsString value;
	dupenv_ret result = dupenv(value, key.c_str());
	if (result == dupenv_ret::NotFound)
	{ 
		return false;
	} else if (result == dupenv_ret::Success) {
		return value;
	} else {	
		return phsnull;
	}
}

i64 StdLib::sys_argc(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "sys_argc");
	return static_cast<i64>(argc);
}

Value StdLib::sys_argv(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "sys_argv");
	if (!args[0].isInt())
		PHS_ERROR("sys_argv() expects an integer as its argument (index)");
	i64 index = args[0].asInt();
	if (index < 0 || index >= argc) 
	{ 
		return phsnull;
	}
	return argv[index];
}

Value StdLib::sys_args(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "sys_args");
	std::vector<Value> arguments;
	arguments.reserve(argc);
	for (int i = 0; i < argc; ++i)
	{
		arguments.emplace_back(argv[i]);
	}
	return Value::createArray(std::move(arguments));
}

Value StdLib::sys_shutdown(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "shutdown");
	if (!args[0].isInt())
		PHS_ERROR("shutdown() expects an integer as its argument (exit code)");
	int ret = static_cast<int>(args[0].asInt());
	vm->setStatus(ret);
	throw VM::Halt();
}

#ifndef SANDBOXED

i64 StdLib::sys_os(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "sys_os");
#if defined(_WIN32)
	return 0;
#elif defined(__linux__)
	return 1;
#elif defined(__APPLE__)
	return 2;
#elif defined(__FreeBSD__)
	return 3;
#elif defined(__unix__)
	return 4;
#else
	return 5;
#endif
}

i64 StdLib::sys_arch(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "sys_arch");
#if defined(TARGET_ARCH_ARM64)
	return 0;
#elif defined(TARGET_ARCH_ARM)
	return 1;
#elif defined(TARGET_ARCH_X64)
	return 2;
#elif defined(TARGET_ARCH_X86)
	return 3;
#else
	return 4;
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
	return {""};
}

Value StdLib::sys_shell(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "sys_shell");
	if (!args[0].isString())
		PHS_ERROR("sys_shell() expects a string as its argument (command)");
	return vm->regRun(OpCode::SYSTEM_R, args[0]);
}

i64 StdLib::sys_fork(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 1, "sys_fork", true);

    if (!args[0].isString())
        PHS_ERROR("sys_fork() expects a string as its first argument (executable)");

    const char *executable = args[0].c_str();
    std::vector<char *> v_argv;

    if (args.size() == 2 && args[1].isArray()) 
    {
        const auto& arr = *args[1].asArray(); 
        
        v_argv.reserve(arr.size());
        
        for (const auto& val : arr) 
        {
            if (!val.isString())
                PHS_ERROR("sys_fork() expects its argument array to contain only strings");
            v_argv.push_back(const_cast<char *>(val.c_str()));
        }
    }
    else 
    {
        int argc = (int)args.size() - 1;
        v_argv.reserve(argc);
        for (int i = 0; i < argc; ++i)
        {
            if (!args[i + 1].isString())
                PHS_ERROR("sys_fork() expects all of its arguments to be strings");
            v_argv.push_back(const_cast<char *>(args[i + 1].c_str()));
        }
    }

    return static_cast<i64>(PHASORstd_sys_run(executable, static_cast<int>(v_argv.size()), v_argv.data()));
}

Value StdLib::sys_fork_output(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "sys_fork_output", true);

	if (!args[0].isString())
		PHS_ERROR("sys_fork_output() expects a string as its first argument (executable)");

	std::vector<std::string> v_args;

	if (args.size() == 2 && args[1].isArray())
	{
		const auto &arr = *args[1].asArray();
		v_args.reserve(arr.size());
		for (const auto &val : arr)
		{
			if (!val.isString())
				PHS_ERROR("sys_fork_output() expects its argument array to contain only strings");
			v_args.push_back(val.stl_string());
		}
	}
	else
	{
		size_t argc = args.size() - 1;
		v_args.reserve(argc);
		for (size_t i = 0; i < argc; ++i)
		{
			if (!args[i + 1].isString())
				PHS_ERROR("sys_fork_output() expects all of its arguments to be strings");
			v_args.push_back(args[i + 1].stl_string());
		}
	}

	auto quoteArg = [](const std::string &s) -> std::string
	{
#ifdef _WIN32
		std::string out = "\"";
		for (char c : s)
		{
			if (c == '\"')
				out += "\\\"";
			else
				out += c;
		}
		out += "\"";
		return out;
#else
		std::string out = "'";
		for (char c : s)
		{
			if (c == '\'')
				out += "'\\''";
			else
				out += c;
		}
		out += "'";
		return out;
#endif
	};

	std::string cmd = quoteArg(args[0].stl_string());
	for (const auto &a : v_args)
	{
		cmd += ' ';
		cmd += quoteArg(a);
	}

#ifdef _WIN32
	std::string fullCmd = "\"" + cmd + "\"";
	FILE *pipe = _popen(fullCmd.c_str(), "r");
#else
	FILE *pipe = popen(cmd.c_str(), "r");
#endif

	if (!pipe)
		PHS_ERROR("sys_fork_output() failed to start process");

	std::string output;
	char buffer[4096];
	size_t n;
	while ((n = fread(buffer, 1, sizeof(buffer), pipe)) > 0)
		output.append(buffer, n);

#ifdef _WIN32
	int rc = _pclose(pipe);
#else
	int status = pclose(pipe);
	int rc = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif

	if (rc != 0)
		return Value(); // null

	return Value(output);
}

i64 StdLib::sys_fork_detached(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "sys_fork_detached", true);

	if (!args[0].isString())
		PHS_ERROR("sys_fork_detached() expects a string as its first argument (executable)");

	const char         *executable = args[0].c_str();
	int                 argc = (int)args.size() - 1;
	std::vector<char *> v_argv(argc);
	for (int i = 0; i < argc; ++i)
	{
		if (!args[i + 1].isString())
			PHS_ERROR("sys_fork_detached() expects all of its arguments to be strings");
		v_argv[i] = const_cast<char *>(args[i + 1].c_str());
	}
	return static_cast<i64>(PHASORstd_sys_run_detached(executable, argc, v_argv.data()));
}

Value StdLib::sys_crash(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "error");
	if (!args[0].isString())
		PHS_ERROR("error() expects a string as its argument (message)");
	vm->reset();
	vm->setStatus(-1);
	PHS_ERROR(args[0].string());
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
