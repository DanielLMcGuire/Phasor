#pragma once

#include <vector>
#include <cstdint>
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
	NativeRuntime(const std::vector<uint8_t> &bytecodeData, const int argc, const char **argv);
	NativeRuntime(const std::string &script, int argc, char **argv);
	~NativeRuntime();
	int  run();
	void addNativeFunction(const std::string &name, void *function);

	static int eval(VM *vm, const std::string &script);

  private:
	static Value runScript(const std::vector<Value> &args, VM *vm); // Run script on independent VM
	Bytecode             m_bytecode;
	std::vector<uint8_t> m_bytecodeData;
	std::string          m_script;
	std::unique_ptr<VM>  m_vm;
	int                  m_argc;
	char               **m_argv;
};

} // namespace Phasor
