// Copyright 2025-2026 Daniel McGuire
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

#include <vector>
#include <phsint.hpp>
#include <string>
#include "../../Codegen/Bytecode/BytecodeSerializer.hpp"
#include "../../Runtime/VM/VM.hpp"

/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/**
 * @class NativeRuntime
 * @brief CLI wrapper for running Phasor scripts and bytecode in-process
 *
 * Allows embedding and running Phasor scripts and bytecode within a native application.
 */
class NativeRuntime
{
  public:
	NativeRuntime(const std::vector<u8> &bytecodeData, const int argc, const char **argv);
	NativeRuntime(Phasor::Bytecode bytecode, const int argc, const char **argv);
	NativeRuntime(std::string script, const int argc, const char **argv);
	NativeRuntime(const Phasor::VM &vm, std::string script, const int argc, const char **argv);
	NativeRuntime(Phasor::VM *vm, const std::vector<u8> &bytecodeData, const int argc, const char **argv);
	~NativeRuntime();
	int                        run();
	int                        runFunctionInt(const std::string& functionName);
	std::optional<std::string> runFunctionString(const std::string& functionName);
	void                       addNativeFunction(const std::string &name, void *function);

	static int eval(VM *vm, const std::string &script);

  private:
	std::shared_ptr<Phasor::VM> m_vm;
	Bytecode                    m_bytecode;
	std::vector<u8>        m_bytecodeData;
	std::string                 m_script;
	int                         m_argc;
	char                      **m_argv;
};

} // namespace Phasor
