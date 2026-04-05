#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <nativeerror.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#pragma comment(lib, "User32.lib")
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

int main(int argc, char *argv[])
{
	try
	{
#ifdef _WIN32
		// Load phasor-runtime.dll
		HMODULE hRuntime = LoadLibraryA("phasorrt.dll");
		if (!hRuntime)
		{
			MessageBoxA(nullptr, "Could not load phasorrt.dll",
			            (std::string(moduleName) + " | Phasor Application - Error").c_str(), MB_OK | MB_ICONERROR);
			std::filesystem::remove(tempFile);
			return 1;
		}

		// Get the exec function
		typedef int(CALLBACK * ExecFunc)(void *, const unsigned char[], size_t, const char *, int, const char **);
		ExecFunc execFunc = (ExecFunc)GetProcAddress(hRuntime, "exec");
		if (!execFunc)
		{
			MessageBoxA(nullptr, "Could not find exec function in DLL",
			            (std::string(moduleName) + " | Phasor Application - Error").c_str(), MB_OK | MB_ICONERROR);
			FreeLibrary(hRuntime);
			std::filesystem::remove(tempFile);
			return 1;
		}

		// Execute the bytecode
		exitCode =
		    execFunc(nullptr, embeddedBytecode, embeddedBytecodeSize, moduleName.c_str(), argc, (const char **)argv);

		// Cleanup
		FreeLibrary(hRuntime);
#else
		// Load phasor-runtime.so
		void *hRuntime = dlopen("libphasorrt.so", RTLD_LAZY);
		if (!hRuntime)
		{
			std::cerr << "Error: Could not load libphasorrt.so: " << dlerror() << "\n";
			std::filesystem::remove(tempFile);
			return 1;
		}

		// Get the exec function
		typedef int (*ExecFunc)(void *, const unsigned char[], size_t, const char *, int, const char **);
		ExecFunc execFunc = (ExecFunc)dlsym(hRuntime, "exec");
		if (!execFunc)
		{
			std::cerr << "Error: Could not find exec function in library\n";
			dlclose(hRuntime);
			std::filesystem::remove(tempFile);
			return 1;
		}

		// Execute the bytecode
		int exitCode = execFunc(nullptr, embeddedBytecode, embeddedBytecodeSize, moduleName.c_str(), argc, (const char **)argv);

		// Cleanup
		dlclose(hRuntime);

		return exitCode;
#endif
	}
	catch (const std::exception &e)
	{
		std::cerr << "Runtime Error: " << e.what() << "\n";
	}
	return 1;
}