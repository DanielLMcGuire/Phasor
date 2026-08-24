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
	vm->registerNativeFunction("sys_fork_detached", StdLib::sys_fork_detached);
	vm->registerNativeFunction("procwait", StdLib::proc_wait);
	vm->registerNativeFunction("procstatus", StdLib::proc_status);
	vm->registerNativeFunction("prockill", StdLib::proc_kill);
	vm->registerNativeFunction("procforget", StdLib::proc_forget);
	vm->registerNativeFunction("procfree", StdLib::proc_free);
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

Value StdLib::sys_fork(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "sys_fork", true);
	if (args.size() > 5)
		PHS_ERROR("sys_fork() expects at most 5 arguments (program, args, pipe, input, isolate)");

	requireString(args[0], "sys_fork", "first argument (program)");
	std::string program = args[0].stl_string();

	std::vector<std::string> procArgs;
	if (args.size() >= 2 && !args[1].isNull())
	{
		if (!args[1].isArray())
			PHS_ERROR("sys_fork() expects an array of strings as its second argument (args)");
		for (const auto &v : *args[1].asArray())
		{
			if (!v.isString())
				PHS_ERROR("sys_fork() expects an array of strings as its second argument (args)");
			procArgs.push_back(v.stl_string());
		}
	}

	bool pipe = false;
	if (args.size() >= 3)
	{
		requireBool(args[2], "sys_fork", "third argument (pipe)");
		pipe = args[2].asBool();
	}

	std::string input;
	if (args.size() >= 4 && !args[3].isNull())
	{
		requireString(args[3], "sys_fork", "fourth argument (input)");
		input = args[3].stl_string();
	}

	bool isolate = false;
	if (args.size() >= 5)
	{
		requireBool(args[4], "sys_fork", "fifth argument (isolate)");
		isolate = args[4].asBool();
	}

	ProcessLaunchOptions opts;
	opts.wantStdin  = pipe && !input.empty();
	opts.wantStdout = pipe;
	opts.isolate    = isolate;

	LaunchedProcess proc;
	try
	{
		proc = launchProcess(program, procArgs, opts);
	}
	catch (const std::exception &)
	{
		return phsnull;
	}

	std::thread stdinWriter;
	if (opts.wantStdin && proc.stdinWrite)
	{
		stdinWriter = std::thread([stream = std::move(proc.stdinWrite), input]() mutable {
			(*stream) << input;
			stream->flush();
			stream.reset();
		});
	}

	std::string output;
	if (pipe && proc.stdoutRead)
	{
		std::stringstream buffer;
		buffer << proc.stdoutRead->rdbuf();
		output = buffer.str();
	}
	if (stdinWriter.joinable())
		stdinWriter.join();

	int exitCode = -1;
#if defined(_WIN32)
	HANDLE nh = static_cast<HANDLE>(proc.nativeHandle);
	WaitForSingleObject(nh, INFINITE);
	DWORD code = 0;
	GetExitCodeProcess(nh, &code);
	CloseHandle(nh);
	exitCode = static_cast<int>(code);
#else
	int status = 0;
	waitpid(static_cast<pid_t>(proc.pid), &status, 0);
	exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif

	Value result = Value::createStruct("ProcessResult");
	result.setField("status", Value(static_cast<i64>(exitCode)));
	result.setField("output", pipe ? Value(output) : phsnull);
	return result;
}

Value StdLib::sys_fork_detached(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "sys_fork_detached", true);
	if (args.size() > 5)
		PHS_ERROR("sys_fork_detached() expects at most 5 arguments (program, args, pipe, stream, isolate)");

	requireString(args[0], "sys_fork_detached", "first argument (program)");
	std::string program = args[0].stl_string();

	std::vector<std::string> procArgs;
	if (args.size() >= 2 && !args[1].isNull())
	{
		if (!args[1].isArray())
			PHS_ERROR("sys_fork_detached() expects an array of strings as its second argument (args)");
		for (const auto &v : *args[1].asArray())
		{
			if (!v.isString())
				PHS_ERROR("sys_fork_detached() expects an array of strings as its second argument (args)");
			procArgs.push_back(v.stl_string());
		}
	}

	bool pipe = false;
	if (args.size() >= 3)
	{
		requireBool(args[2], "sys_fork_detached", "third argument (pipe)");
		pipe = args[2].asBool();
	}

	i64 stream = 1;
	if (args.size() >= 4 && !args[3].isNull())
	{
		requireInt(args[3], "sys_fork_detached", "fourth argument (stream)");
		stream = args[3].asInt();
		if (stream < 0 || stream > 3)
			PHS_ERROR("sys_fork_detached() stream must be 0 (stdin), 1 (stdout), 2 (stderr), or 3 (stdout+stderr)");
	}

	bool isolate = false;
	if (args.size() >= 5)
	{
		requireBool(args[4], "sys_fork_detached", "fifth argument (isolate)");
		isolate = args[4].asBool();
	}

	ProcessLaunchOptions opts;
	if (pipe)
	{
		opts.wantStdin  = (stream == 0);
		opts.wantStdout = (stream == 1 || stream == 3);
		opts.wantStderr = (stream == 2 || stream == 3);
	}
	opts.isolate = isolate;

	LaunchedProcess proc;
	try
	{
		proc = launchProcess(program, procArgs, opts);
	}
	catch (const std::exception &)
	{
		return phsnull;
	}

	auto handle = std::make_unique<ProcessHandle>();
#if defined(_WIN32)
	handle->nativeHandle = proc.nativeHandle;
	handle->processId    = proc.processId;
#else
	handle->pid = proc.pid;
#endif
	handle->isolated = isolate;
	i64 poolHandle = allocProcessHandle(std::move(handle));

	Value result = Value::createStruct("ProcessHandle");
#if defined(_WIN32)
	result.setField("pid", Value(static_cast<i64>(proc.processId)));
#else
	result.setField("pid", Value(static_cast<i64>(proc.pid)));
#endif
	result.setField("handle", Value(poolHandle));
	result.setField("stdin",  proc.stdinWrite ? Value(allocFileDescriptor(std::move(proc.stdinWrite), StreamKind::Pipe)) : phsnull);
	result.setField("stdout", proc.stdoutRead ? Value(allocFileDescriptor(std::move(proc.stdoutRead), StreamKind::Pipe)) : phsnull);
	result.setField("stderr", proc.stderrRead ? Value(allocFileDescriptor(std::move(proc.stderrRead), StreamKind::Pipe)) : phsnull);
	return result;
}

i64 StdLib::proc_wait(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "procwait");
	requireInt(args[0], "procwait", "argument (process handle)");
	i64 h = args[0].asInt();

	std::unique_lock<std::mutex> lock(getProcessPoolMutex());
	auto *proc = getProcessHandleLocked(h);
	if (!proc) PHS_ERROR("procwait() invalid process handle");

	if (!proc->exited)
	{
#if defined(_WIN32)
		HANDLE nh = static_cast<HANDLE>(proc->nativeHandle);
		lock.unlock();
		WaitForSingleObject(nh, INFINITE);
		lock.lock();
		proc = getProcessHandleLocked(h);
		if (!proc) PHS_ERROR("procwait() process handle was freed while waiting");
		DWORD code = 0;
		GetExitCodeProcess(nh, &code);
		proc->exitCode = static_cast<int>(code);
#else
		long pid = proc->pid;
		lock.unlock();
		int status = 0;
		pid_t wr = waitpid(static_cast<pid_t>(pid), &status, 0);
		lock.lock();
		proc = getProcessHandleLocked(h);
		if (!proc) PHS_ERROR("procwait() process handle was freed while waiting");
		if (wr > 0) proc->exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif
		proc->exited = true;
	}
	return static_cast<i64>(proc->exitCode);
}

Value StdLib::proc_status(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "procstatus");
	requireInt(args[0], "procstatus", "argument (process handle)");

	std::lock_guard<std::mutex> lock(getProcessPoolMutex());
	auto *proc = getProcessHandleLocked(args[0].asInt());
	if (!proc) PHS_ERROR("procstatus() invalid process handle");
	if (!pollProcessExitLocked(*proc)) return phsnull;
	return Value(static_cast<i64>(proc->exitCode));
}

bool StdLib::proc_kill(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "prockill", true);
	if (args.size() > 2) PHS_ERROR("prockill() expects at most 2 arguments (handle, signal)");
	requireInt(args[0], "prockill", "first argument (process handle)");
	i64 sig = 15;
	if (args.size() == 2) { requireInt(args[1], "prockill", "second argument (signal)"); sig = args[1].asInt(); }

	std::lock_guard<std::mutex> lock(getProcessPoolMutex());
	auto *proc = getProcessHandleLocked(args[0].asInt());
	if (!proc) PHS_ERROR("prockill() invalid process handle");
	if (proc->exited) return false;

#if defined(_WIN32)
	HANDLE nh = static_cast<HANDLE>(proc->nativeHandle);
	if (sig == 9 || !proc->isolated)
		return TerminateProcess(nh, 1) != 0;
	return GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, proc->processId) != 0;
#else
	pid_t target = proc->isolated ? -static_cast<pid_t>(proc->pid) : static_cast<pid_t>(proc->pid);
	return ::kill(target, static_cast<int>(sig)) == 0;
#endif
}

bool StdLib::proc_forget(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "procforget");
	requireInt(args[0], "procforget", "argument (process handle)");
	i64 h = args[0].asInt();

	std::lock_guard<std::mutex> lock(getProcessPoolMutex());
	auto *proc = getProcessHandleLocked(h);
	if (!proc) return false;

	if (pollProcessExitLocked(*proc))
		releaseProcessHandleLocked(h);
	else
		proc->forgotten = true;
	return true;
}

bool StdLib::proc_free(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "procfree");
	requireInt(args[0], "procfree", "argument (process handle)");
	i64 h = args[0].asInt();

	std::unique_lock<std::mutex> lock(getProcessPoolMutex());
	auto *proc = getProcessHandleLocked(h);
	if (!proc) return false;

	if (!proc->exited)
	{
#if defined(_WIN32)
		HANDLE nh = static_cast<HANDLE>(proc->nativeHandle);
		lock.unlock();
		WaitForSingleObject(nh, INFINITE);
		lock.lock();
#else
		long pid = proc->pid;
		lock.unlock();
		int status = 0;
		waitpid(static_cast<pid_t>(pid), &status, 0);
		lock.lock();
#endif
		proc = getProcessHandleLocked(h);
		if (!proc) return true;
	}
	releaseProcessHandleLocked(h);
	return true;
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
