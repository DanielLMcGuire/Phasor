#include "system.h"

size_t PHASORstd_sys_getAvailableMemory()
{
#ifdef _WIN32
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof(statex);
	if (!GlobalMemoryStatusEx(&statex))
		return 0;
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
	if (sysctlbyname("hw.pagesize", &page_size, &len, NULL, 0) != 0 || page_size == 0)
		return 0;

	long free_pages = 0;
#if defined(__OpenBSD__)
	len = sizeof(free_pages);
	if (sysctlbyname("uvm.stats.sys.free", &free_pages, &len, NULL, 0) != 0)
		return 0;
#else
	len = sizeof(free_pages);
	if (sysctlbyname("vm.stats.vm.v_free_count", &free_pages, &len, NULL, 0) != 0)
		return 0;
#endif

	return (size_t)free_pages * (size_t)page_size;

#else
	return 0;
#endif
}

int PHASORstd_sys_run(const char *name, int argc, char **argv)
{
	char **args = (char **)malloc((argc + 2) * sizeof(char *));
	if (!args)
		return -1;
	 
	args[0] = (char *)name;
	for (int i = 0; i < argc; i++)
		args[i + 1] = argv[i];
	args[argc + 1] = NULL;

#ifdef _WIN32
	int ret = (int)_spawnvp(_P_WAIT, name, (const char *const *)args);
	if (ret == -1)
	{
		perror(name);
	}
#else
	int   ret = -1;
	pid_t pid = fork();
	if (pid == 0)
	{
		execvp(name, args);
		exit(1); // execvp failed
	}
	int status;
	waitpid(pid, &status, 0);
	ret = WEXITSTATUS(status);
#endif

	free(args);
	return ret;
}