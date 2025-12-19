#include <Shell.hpp>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>


#include "../../Runtime/Value.hpp"

// Forward declare native runtime entry points (linked in from the runtime library)
extern "C" void exec(const unsigned char embeddedBytecode[], size_t embeddedBytecodeSize,
	                   const char *moduleName, const void *nativeFunctionsVector = nullptr);

// Main entry point
int main(int argc, char *argv[], char *envp[])
{
	int         exitCode = 1;

	try
	{
		exec(embeddedBytecode, embeddedBytecodeSize, moduleName.c_str());
		exitCode = 0;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Runtime Error: " << e.what() << "\n";
		return 1;
	}

	return exitCode;
}
