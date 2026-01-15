#ifndef CORE_SYSTEM_H
#define CORE_SYSTEM_H	

#ifdef _WIN32
#include <windows.h>
#else
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