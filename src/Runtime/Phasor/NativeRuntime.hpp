#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include "../../Codegen/Bytecode/BytecodeSerializer.hpp"
#include "../../Runtime/VM/VM.hpp"

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
	NativeRuntime(const std::vector<uint8_t> &bytecodeData, const int argc, const char **argv);
	NativeRuntime(const std::string &script, int argc, char **argv);
	~NativeRuntime();
	int  run();
	void addNativeFunction(const std::string &name, void *function);

	static void eval(VM *vm, const std::string &script);

  private:
	Bytecode             m_bytecode;
	std::vector<uint8_t> m_bytecodeData;
	std::string          m_script;
	std::unique_ptr<VM>  m_vm;
	int                  m_argc;
	char               **m_argv;
};

} // namespace Phasor
