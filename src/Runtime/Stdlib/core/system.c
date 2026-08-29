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

#include "system.h"

#if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#  ifndef WIFEXITED
#    define WIFEXITED(s)   (((s) & 0x7F) == 0)
#  endif
#  ifndef WEXITSTATUS
#    define WEXITSTATUS(s) (((s) >> 8) & 0xFF)
#  endif
#endif

[[nodiscard]] size_t PHASORstd_sys_getAvailableMemory(void)
{
#ifdef _WIN32
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	if (!GlobalMemoryStatusEx(&statex)) 
	{
		return 0;
	}
	return (size_t)statex.ullAvailPhys;

#elif defined(__APPLE__) && defined(__MACH__)
	mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
	vm_statistics_data_t   vmstat;
	if (host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vmstat, &count) != KERN_SUCCESS)
		return 0;

	vm_size_t page_size = 0;
	if (host_page_size(mach_host_self(), &page_size) != KERN_SUCCESS || page_size == 0)
		return 0;

	natural_t available_pages = vmstat.free_count + vmstat.inactive_count;
	return (size_t)available_pages * (size_t)page_size;

#elif defined(__linux__)
	long pages = sysconf(_SC_AVPHYS_PAGES);
	long page_size = sysconf(_SC_PAGESIZE);
	if (pages == -1 || page_size == -1)
		return 0;
	return (size_t)pages * (size_t)page_size;

#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
	size_t page_size = 0;
	size_t len = sizeof(page_size);
	if (sysctlbyname("hw.pagesize", &page_size, &len, nullptr, 0) != 0 || page_size == 0)
		return 0;

	long free_pages = 0;
#if defined(__OpenBSD__)
	len = sizeof(free_pages);
	if (sysctlbyname("uvm.stats.sys.free", &free_pages, &len, nullptr, 0) != 0)
		return 0;
#else
	len = sizeof(free_pages);
	if (sysctlbyname("vm.stats.vm.v_free_count", &free_pages, &len, nullptr, 0) != 0)
		return 0;
#endif

	return (size_t)free_pages * (size_t)page_size;

#else
	return 0;
#endif
}
