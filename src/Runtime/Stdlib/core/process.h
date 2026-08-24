#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "pipe_streambuf.h"

namespace Phasor
{

struct ProcessLaunchOptions
{
	bool wantStdin  = false;
	bool wantStdout = false;
	bool wantStderr = false;
	bool isolate    = false;
};

struct LaunchedProcess
{
#if defined(_WIN32)
	void*         nativeHandle = nullptr;
	unsigned long processId    = 0;
#else
	long          pid = -1;
#endif
	std::unique_ptr<std::iostream> stdinWrite;
	std::unique_ptr<std::iostream> stdoutRead;
	std::unique_ptr<std::iostream> stderrRead;
};

LaunchedProcess launchProcess(const std::string &program,
                               const std::vector<std::string> &args,
                               const ProcessLaunchOptions &opts);

} // namespace Phasor