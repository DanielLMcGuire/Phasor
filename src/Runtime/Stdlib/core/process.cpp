#include "process.h"
#include <stdexcept>

#if defined(_WIN32)
	#include <windows.h>
#else
	#include <unistd.h>
	#include <fcntl.h>
	#include <spawn.h>
	extern char **environ;
#endif

namespace Phasor
{

#if defined(_WIN32)

namespace {
std::string quoteWindowsArg(const std::string &s)
{
	if (!s.empty() && s.find_first_of(" \t\n\v\"") == std::string::npos)
		return s;

	std::string out = "\"";
	for (auto it = s.begin();; ++it)
	{
		size_t backslashes = 0;
		while (it != s.end() && *it == '\\') { ++backslashes; ++it; }
		if (it == s.end())
		{
			out.append(backslashes * 2, '\\');
			break;
		}
		if (*it == '\"')
		{
			out.append(backslashes * 2 + 1, '\\');
			out += '\"';
		}
		else
		{
			out.append(backslashes, '\\');
			out += *it;
		}
	}
	out += '\"';
	return out;
}
} // namespace

LaunchedProcess launchProcess(const std::string &program, const std::vector<std::string> &args,
                               const ProcessLaunchOptions &opts)
{
	LaunchedProcess result;
	SECURITY_ATTRIBUTES sa{ sizeof(sa), nullptr, TRUE };

	HANDLE stdinRead = nullptr, stdinWrite = nullptr;
	HANDLE stdoutRead = nullptr, stdoutWrite = nullptr;
	HANDLE stderrRead = nullptr, stderrWrite = nullptr;

	auto makePipe = [&](HANDLE &readEnd, HANDLE &writeEnd, bool parentKeepsRead)
	{
		if (!CreatePipe(&readEnd, &writeEnd, &sa, 0))
			throw std::runtime_error("CreatePipe failed");
		SetHandleInformation(parentKeepsRead ? readEnd : writeEnd, HANDLE_FLAG_INHERIT, 0);
	};

	if (opts.wantStdin)  makePipe(stdinRead, stdinWrite, false);
	if (opts.wantStdout) makePipe(stdoutRead, stdoutWrite, true);
	if (opts.wantStderr) makePipe(stderrRead, stderrWrite, true);

	STARTUPINFOA si{};
	si.cb = sizeof(si);
	bool redirect = opts.wantStdin || opts.wantStdout || opts.wantStderr;
	if (redirect)
	{
		si.dwFlags |= STARTF_USESTDHANDLES;
		si.hStdInput  = opts.wantStdin  ? stdinRead   : GetStdHandle(STD_INPUT_HANDLE);
		si.hStdOutput = opts.wantStdout ? stdoutWrite : GetStdHandle(STD_OUTPUT_HANDLE);
		si.hStdError  = opts.wantStderr ? stderrWrite : GetStdHandle(STD_ERROR_HANDLE);
	}

	std::string cmdLine = quoteWindowsArg(program);
	for (const auto &a : args) { cmdLine += ' '; cmdLine += quoteWindowsArg(a); }
	std::vector<char> cmdLineBuf(cmdLine.begin(), cmdLine.end());
	cmdLineBuf.push_back('\0');

	DWORD creationFlags = opts.isolate ? CREATE_NEW_PROCESS_GROUP : 0;

	PROCESS_INFORMATION pi{};
	BOOL ok = CreateProcessA(nullptr, cmdLineBuf.data(), nullptr, nullptr,
	                          redirect ? TRUE : FALSE, creationFlags,
	                          nullptr, nullptr, &si, &pi);

	if (opts.wantStdin)  CloseHandle(stdinRead);
	if (opts.wantStdout) CloseHandle(stdoutWrite);
	if (opts.wantStderr) CloseHandle(stderrWrite);

	if (!ok)
	{
		if (opts.wantStdin)  CloseHandle(stdinWrite);
		if (opts.wantStdout) CloseHandle(stdoutRead);
		if (opts.wantStderr) CloseHandle(stderrRead);
		throw std::runtime_error("CreateProcess failed for: " + program);
	}

	CloseHandle(pi.hThread);
	result.nativeHandle = pi.hProcess;
	result.processId    = pi.dwProcessId;

	if (opts.wantStdin)
		result.stdinWrite = std::make_unique<OwningIOStream>(std::make_unique<NativePipeStreamBuf>(stdinWrite, false));
	if (opts.wantStdout)
		result.stdoutRead = std::make_unique<OwningIOStream>(std::make_unique<NativePipeStreamBuf>(stdoutRead, true));
	if (opts.wantStderr)
		result.stderrRead = std::make_unique<OwningIOStream>(std::make_unique<NativePipeStreamBuf>(stderrRead, true));

	return result;
}

#else // POSIX

LaunchedProcess launchProcess(const std::string &program, const std::vector<std::string> &args,
                               const ProcessLaunchOptions &opts)
{
	LaunchedProcess result;
	int stdinFds[2]{-1,-1}, stdoutFds[2]{-1,-1}, stderrFds[2]{-1,-1};

	auto makePipe = [](int fds[2]) { if (::pipe(fds) != 0) throw std::runtime_error("pipe() failed"); };
	if (opts.wantStdin)  makePipe(stdinFds);
	if (opts.wantStdout) makePipe(stdoutFds);
	if (opts.wantStderr) makePipe(stderrFds);

	posix_spawn_file_actions_t actions;
	posix_spawn_file_actions_init(&actions);
	if (opts.wantStdin)
	{
		posix_spawn_file_actions_adddup2(&actions, stdinFds[0], STDIN_FILENO);
		posix_spawn_file_actions_addclose(&actions, stdinFds[1]);
	}
	if (opts.wantStdout)
	{
		posix_spawn_file_actions_adddup2(&actions, stdoutFds[1], STDOUT_FILENO);
		posix_spawn_file_actions_addclose(&actions, stdoutFds[0]);
	}
	if (opts.wantStderr)
	{
		posix_spawn_file_actions_adddup2(&actions, stderrFds[1], STDERR_FILENO);
		posix_spawn_file_actions_addclose(&actions, stderrFds[0]);
	}

	std::vector<char*> argv;
	argv.push_back(const_cast<char*>(program.c_str()));
	for (const auto &a : args) argv.push_back(const_cast<char*>(a.c_str()));
	argv.push_back(nullptr);

	posix_spawnattr_t attr;
	posix_spawnattr_t *attrp = nullptr;
	if (opts.isolate)
	{
		posix_spawnattr_init(&attr);
		posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETPGROUP);
		posix_spawnattr_setpgroup(&attr, 0);
		attrp = &attr;
	}

	pid_t pid = -1;
	int rc = posix_spawnp(&pid, program.c_str(), &actions, attrp, argv.data(), environ);
	if (opts.isolate) posix_spawnattr_destroy(&attr);

	if (opts.wantStdin)  ::close(stdinFds[0]);
	if (opts.wantStdout) ::close(stdoutFds[1]);
	if (opts.wantStderr) ::close(stderrFds[1]);

	if (rc != 0)
	{
		if (opts.wantStdin)  ::close(stdinFds[1]);
		if (opts.wantStdout) ::close(stdoutFds[0]);
		if (opts.wantStderr) ::close(stderrFds[0]);
		throw std::runtime_error("posix_spawnp failed for: " + program);
	}

	result.pid = pid;
	if (opts.wantStdin)
	{
		::fcntl(stdinFds[1], F_SETFD, FD_CLOEXEC);
		result.stdinWrite = std::make_unique<OwningIOStream>(std::make_unique<NativePipeStreamBuf>(stdinFds[1], false));
	}
	if (opts.wantStdout)
	{
		::fcntl(stdoutFds[0], F_SETFD, FD_CLOEXEC);
		result.stdoutRead = std::make_unique<OwningIOStream>(std::make_unique<NativePipeStreamBuf>(stdoutFds[0], true));
	}
	if (opts.wantStderr)
	{
		::fcntl(stderrFds[0], F_SETFD, FD_CLOEXEC);
		result.stderrRead = std::make_unique<OwningIOStream>(std::make_unique<NativePipeStreamBuf>(stderrFds[0], true));
	}

	return result;
}

#endif

} // namespace Phasor