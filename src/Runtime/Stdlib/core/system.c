#include "system.h"

size_t getAvailableMemory()
#ifdef _WIN32
{
	// WIN32
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	if (!GlobalMemoryStatusEx(&statex))
		return 0;
	return (size_t)statex.ullAvailPhys;
}
#else
{
	// POSIX
	long pages = sysconf(_SC_AVPHYS_PAGES);
	long page_size = sysconf(_SC_PAGESIZE);
	if (pages == -1 || page_size == -1)
		return 0;
	return (size_t)pages * (size_t)page_size;
}
#endif