#ifndef CMAKE_PCH
#include "VM.hpp"
#endif

#ifdef PHASOR_USES_BOOST
	#include <boost/assert/source_location.hpp>
	#define PHS_SRC_LOC() (std::format("{} @ {}:{}", BOOST_CURRENT_LOCATION.function_name(), BOOST_CURRENT_LOCATION.file_name(), BOOST_CURRENT_LOCATION.line()))
#else
	#define PHS_SRC_LOC() (std::format("VM::{}()", __func__))
#endif

namespace Phasor
{

VM::VM() : stack(&stack_pool)
{
#ifdef TRACING
	log(std::format("{}: v{}:\nnormal instance created {:#x}\n", PHS_SRC_LOC(), getVersion(),
	                (uintptr_t)this));
	log(std::format("Value size: {}, VM Size: {}\n", sizeof(Phasor::Value), sizeof(Phasor::VM)));
	flush();
#endif
}

VM::VM(const Bytecode &bytecode) : stack(&stack_pool)
{
#ifdef TRACING
	log(std::format("{}: v{}:\nfast instance created {:#x}\n", PHS_SRC_LOC(), getVersion(), (uintptr_t)this));
	flush();
#endif
	run(bytecode);
}

VM::VM(const OpCode &op, const int &operand1, const int &operand2, const int &operand3) : stack(&stack_pool)
{
#ifdef TRACING
	log(std::format("{}: v{}:\noperation instance created {:#x}\n", PHS_SRC_LOC(), getVersion(),
	                (uintptr_t)this));
	flush();
#endif
	operation(op, operand1, operand2, operand3);
}

VM::~VM()
{
	cleanup();
#ifdef TRACING
	log(std::format("{}: killed {:#x}\n", PHS_SRC_LOC(), (uintptr_t)this));
	flush();
#endif
}
} // namespace Phasor