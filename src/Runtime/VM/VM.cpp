#ifndef CMAKE_PCH
#include "VM.hpp"
#endif

namespace Phasor
{

VM::VM()
{
#ifdef TRACING
	log(std::format("Phasor::VM::{}(): v{}:\nnormal instance created {:#x}\n", __func__, getVersion(), (uintptr_t)this));
	flush();
#endif
}

VM::VM(const Bytecode &bytecode)
{
#ifdef TRACING
	log(std::format("Phasor::VM::{}(): v{}:\nfast instance created {:#x}\n", __func__, getVersion(),(uintptr_t)this));
	flush();
#endif
	run(bytecode);
}

VM::VM(const OpCode &op, const int &operand1, const int &operand2, const int &operand3)
{
#ifdef TRACING
	log(std::format("Phasor::VM::{}(): v{}:\noperation instance created {:#x}\n", __func__, getVersion(), (uintptr_t)this));
	flush();
#endif
	operation(op, operand1, operand2, operand3);

}

VM::~VM()
{
	cleanup();
#ifdef TRACING
	log(std::format("Phasor::VM::{}(): deconstructed {:#x}\n", __func__, (uintptr_t)this));
	flush();
#endif
}
} // namespace Phasor