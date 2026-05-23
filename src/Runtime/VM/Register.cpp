#ifndef CMAKE_PCH
#include "VM.hpp"
#endif
#include <phsint.hpp>

namespace Phasor
{

void VM::setRegister(const u8 index, const Value &value)
{
#ifdef TRACING
	log(std::format("VM::{}(r{}, {:T})\n", __func__, index, value));
	flush();
#endif
	registers[index] = value;
}

void VM::freeRegister(const u8 index)
{
#ifdef TRACING
	log(std::format("VM::{}(r{})\n", __func__, index));
	flush();
#endif
	registers[index] = Value();
}

Value VM::getRegister(const u8 index)
{
#ifdef TRACING
	log(std::format("VM::{}(r{}) -> {:T}\n", __func__, index, registers[index]));
	flush();
#endif
	return registers[index];
}

size_t VM::getRegisterCount()
{
#ifdef TRACING
	log(std::format("VM::{}() -> {}\n", __func__, registers.size()));
	flush();
#endif
	return registers.size();
}

} // namespace Phasor