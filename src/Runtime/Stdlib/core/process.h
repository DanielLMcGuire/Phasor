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

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "pipe_streambuf.h"

// Phasor stdlibcore system/process

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