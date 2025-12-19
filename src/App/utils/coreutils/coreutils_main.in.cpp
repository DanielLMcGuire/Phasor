//   coreutils_main -- wrapper to compile phs coreutils as native applications.
//   Implementation of '@tgt@'
//
//   Copyright (C) 2025 Daniel McGuire.
//
//   This program is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>

#include "@HEADER@"
#include "@CMAKE_SOURCE_DIR@/src/Runtime/Value.hpp"

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
