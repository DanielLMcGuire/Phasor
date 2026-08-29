// Copyright 2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://phasor.pages.dev/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <PhasorString.hpp>
#include <vector>
#include <filesystem>
/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/**
 * @class CCompiler
 * @brief CLI wrapper for C++ code generation from Phasor source
 *
 * Compiles Phasor source files to C++ source files that embed bytecode
 * and link against the phasor-runtime DLL.
 */
class CCompiler
{
  public:
	CCompiler(int argc, char *argv[]);
	int run();

  private:
	struct Args
	{
		std::filesystem::path              inputFile;
		std::filesystem::path              outputFile;
		std::filesystem::path              mainFile;
		std::vector<std::filesystem::path> includePaths;
		std::vector<Phasor::string>        defines;
		Phasor::string                     moduleName;
		bool                               verbose = false;
		bool                               showHelp = false;
		Phasor::string                     compiler;
		Phasor::string                     linker;
		bool                               run = false;
		bool                               headerOnly = false;
		bool                               objectOnly = false;
		bool                               generateOnly = false;
	} m_args;

	bool parseArguments(int argc, char *argv[]);
	static bool showHelp(const Phasor::string &programName);
	bool generateHeader(const std::filesystem::path &sourcePath, const std::filesystem::path &outputPath) const;
	static bool generateSource(const std::filesystem::path &sourcePath, const std::filesystem::path &outputPath);
	bool compileSource(const std::filesystem::path &sourcePath, const std::filesystem::path &outputPath);
	bool linkObject(const std::filesystem::path &objectPath, const std::filesystem::path &outputPath);
};

} // namespace Phasor