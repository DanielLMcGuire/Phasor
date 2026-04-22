#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <Value.hpp>

extern "C" int exec(void *state, const unsigned char embeddedBytecode[], size_t embeddedBytecodeSize,
                    const char *moduleName, int argc, const char *argv[]);

int main(int argc, char *argv[])
{
	try
	{
		return exec(nullptr, embeddedBytecode, embeddedBytecodeSize, moduleName.c_str(), argc, (const char **)argv);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Runtime Error: " << e.what() << "\n";
	}

	return 1;
}