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

#include <string>
#include <filesystem>
/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/**
 * @class Disassembler
 * @brief CLI wrapper for dissassembling Phasor binaries
 */
class Disassembler
{
  public:
	Disassembler(int argc, char *argv[]);
	int run();

  private:
	struct Args
	{
		std::filesystem::path inputFile;
		std::filesystem::path outputFile;
		std::filesystem::path program;
		bool                  showHelp = false;
		bool                  silent = false;
	} m_args;

	bool parseArguments(int argc, char *argv[]);
	void showHelp() const;
	bool decompileBinary();
};

} // namespace Phasor
