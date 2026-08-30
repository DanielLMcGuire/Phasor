// Copyright 2025-2026 Daniel McGuire
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

#ifndef CORE_SYSTEM_H
#define CORE_SYSTEM_H

// Phasor stdlibcore system

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <unistd.h>
#include <mach/mach.h>
#include <mach/host_info.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <sys/wait.h>
#endif

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

	size_t PHASORstd_sys_getAvailableMemory(void);

#ifdef __cplusplus
}
#endif
#endif // CORE_SYSTEM_H
