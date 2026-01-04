#include "../../CLI/Runtime/NativeRuntime.hpp"

#ifdef _WIN32
#include <windows.h>
#ifdef _SHARED
#define DLLEXPORT __declspec(dllexport)
#endif
#else
#include <iostream>
#ifdef _SHARED
#define DLLEXPORT __attribute__((visibility("default")))
#endif
#endif
#ifndef _SHARED
#define DLLEXPORT
#endif

#include <vector>
#include <cstring>
#include <sstream>

std::vector<std::string> splits(const std::string &input)
{
	std::vector<std::string> parts;

	std::istringstream iss(input);
	std::string        word;
	while (iss >> word)
	{
		parts.push_back(word);
	}
	return parts;
}

extern "C"
{
	DLLEXPORT void exec(const unsigned char embeddedBytecode[], size_t embeddedBytecodeSize, const char *moduleName,
	                    const void *nativeFunctionsVector)
	{
		try
		{
			std::vector<uint8_t>  bytecodeData(embeddedBytecode, embeddedBytecode + embeddedBytecodeSize);
			Phasor::NativeRuntime NativeRT(bytecodeData);

			if (nativeFunctionsVector != nullptr)
			{
				const void* ptr = nativeFunctionsVector;
				const auto* funcVector = static_cast<const std::vector<std::pair<std::string, void*>>*>(ptr);
				for (const auto &nativeFunction : *funcVector)
				{
					NativeRT.addNativeFunction(nativeFunction.first, nativeFunction.second);
				}
			}

			NativeRT.run();
		}
		catch (const std::exception &e)
		{
#ifdef _WIN32
			MessageBoxA(nullptr, e.what(), (std::string(moduleName) + " | Phasor Runtime - Error").c_str(),
			            MB_OK | MB_ICONERROR);
#else
			std::cerr << "Error: " << e.what() << "\n\a";
#endif
		}
	}

	DLLEXPORT void jitExec(const char *script, const char *moduleName, const void *nativeFunctionsVector)
	{
		try
		{
			Phasor::NativeRuntime NativeRT(script);

			if (nativeFunctionsVector != nullptr)
			{
				const void* ptr = nativeFunctionsVector;
				const auto* funcVector = static_cast<const std::vector<std::pair<std::string, void*>>*>(ptr);
				for (const auto &nativeFunction : *funcVector)
				{
					NativeRT.addNativeFunction(nativeFunction.first, nativeFunction.second);
				}
			}

			NativeRT.run();
		}
		catch (const std::exception &e)
		{
#ifdef _WIN32
			MessageBoxA(nullptr, e.what(), (std::string(moduleName) + " | Phasor Runtime - Error").c_str(),
			            MB_OK | MB_ICONERROR);
#else
			std::cerr << "Error: " << e.what() << "\n\a";
#endif
		}
	}

	DLLEXPORT void executeScript(void *vm, const char *script)
	{
		if (vm == nullptr)
		{
			return;
		}

		VM *phasorVM = (VM *)vm;
		Phasor::NativeRuntime::eval(phasorVM, script);
	}
}