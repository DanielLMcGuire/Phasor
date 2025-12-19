#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include "../../Codegen/Bytecode/BytecodeSerializer.hpp"
#include "../../Runtime/VM/VM.hpp"

namespace Phasor
{

class NativeRuntime
{
  public:
	NativeRuntime(const std::vector<uint8_t> &bytecodeData);
	NativeRuntime(const std::string &script);
	~NativeRuntime();
	int  run();
	void addNativeFunction(const std::string &name, void *function);

	static void eval(VM *vm, const std::string &script);

  private:
	Bytecode             m_bytecode;
	std::vector<uint8_t> m_bytecodeData;
	std::string          m_script;
	std::unique_ptr<VM>  m_vm;
};

} // namespace Phasor
