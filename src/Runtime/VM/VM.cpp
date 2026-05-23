#ifndef CMAKE_PCH
#include "VM.hpp"
#endif

#include "JIT/PhasorJIT.hpp"

namespace Phasor
{

VM::VM() : stack_pool(), stack(&stack_pool)
{
	jit = std::make_unique<PhasorJIT>();
#ifdef TRACING
	log(std::format("Phasor::VM::{}(): v{}:\nnormal instance created {:#x}\n", __func__, getVersion(),
	                (uintptr_t)this));
	log(std::format("Value size: {}, VM Size: {}\n", sizeof(Phasor::Value), sizeof(Phasor::VM)));
	flush();
#endif
}

VM::VM(const Bytecode &bytecode) : stack_pool(), stack(&stack_pool)
{
	jit = std::make_unique<PhasorJIT>();
#ifdef TRACING
	log(std::format("Phasor::VM::{}(): v{}:\nfast instance created {:#x}\n", __func__, getVersion(), (uintptr_t)this));
	flush();
#endif
	run(bytecode);
}

VM::VM(const OpCode &op, const int &operand1, const int &operand2, const int &operand3) : stack_pool(), stack(&stack_pool)
{
	jit = std::make_unique<PhasorJIT>();
#ifdef TRACING
	log(std::format("Phasor::VM::{}(): v{}:\noperation instance created {:#x}\n", __func__, getVersion(),
	                (uintptr_t)this));
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