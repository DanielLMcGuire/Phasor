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
// limitations under the License.#pragma once

#include <string>
#include <vector>
/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/**
 * @class BinaryRuntime
 * @brief CLI wrapper for running Phasor bytecode binaries
 *
 * Loads and executes Phasor bytecode binaries (.phsb files).
 */
class BinaryRuntime
{
  public:
	BinaryRuntime(int argc, char *argv[]);
	int run() const;

  private:
	struct Args
	{
		std::string inputFile;
		bool        verbose = false;
		int         scriptArgc = 0;
		char      **scriptArgv = nullptr;
	} m_args;

	void parseArguments(int argc, char *argv[]);
	static void showHelp(const std::string &programName);
};

} // namespace Phasor
