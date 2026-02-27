

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
extern "C" void exec(const unsigned char embeddedBytecode[], size_t embeddedBytecodeSize, const char *moduleName,
                     const void *nativeFunctionsVector, const int argc, const char *argv[]);

// Main entry point
int main(int argc, char *argv[])
{
	int exitCode = 1;

	try
	{
		exec(embeddedBytecode, embeddedBytecodeSize, moduleName.c_str(), nullptr, argc, argv);
		exitCode = 0;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Runtime Error: " << e.what() << "\n";
		return 1;
	}

	return exitCode;
}
