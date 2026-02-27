// #include "yourheader.h"
// PROVIDES:
// inline const unsigned char embeddedBytecode[]
// inline const size_t embeddedBytecodeSize
// std::string moduleName

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#pragma comment(lib, "User32.lib")
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

// New functionality: Pass bytecode and size as raw args to exec to both windows and other

// Main entry point
int main(int argc, char *argv[])
{
	std::string tempFile;
	int         exitCode = 1;

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
		typedef void(CALLBACK * ExecFunc)(const unsigned char[], size_t, const char *, const void *);
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
		execFunc(embeddedBytecode, embeddedBytecodeSize, moduleName.c_str(), nullptr);
		exitCode = 0;

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
		typedef void (*ExecFunc)(const char *, size_t, const char *, const void *);
		ExecFunc execFunc = (ExecFunc)dlsym(hRuntime, "exec");
		if (!execFunc)
		{
			std::cerr << "Error: Could not find exec function in library\n";
			dlclose(hRuntime);
			std::filesystem::remove(tempFile);
			return 1;
		}

		// Execute the bytecode
		execFunc(tempFile.c_str(), tempFile.size(), moduleName.c_str(), nullptr);
		exitCode = 0;

		// Cleanup
		dlclose(hRuntime);
#endif

		// Remove temporary file
		std::filesystem::remove(tempFile);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Runtime Error: " << e.what() << "\n";
		if (!tempFile.empty())
		{
			std::filesystem::remove(tempFile);
		}
		return 1;
	}

	return exitCode;
}
