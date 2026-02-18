#ifndef CORE_SYSTEM_H
#define CORE_SYSTEM_H

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <mach/mach.h>
#include <mach/host_info.h>
#elif defined(__linux__)
#include <unistd.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif
	size_t getAvailableMemory();
#ifdef __cplusplus
}
#endif
#endif // CORE_SYSTEM_H