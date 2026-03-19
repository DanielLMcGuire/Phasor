#ifndef CMAKE
#include "VM.hpp"
#endif

namespace Phasor
{

void VM::setRegister(const uint8_t index, const Value &value)
{
#ifdef TRACING
	log(std::format("{}(r{}, {:T})\n", __func__, index, value));
	flush();
#endif
	registers[index] = value;
}

void VM::freeRegister(const uint8_t index)
{
#ifdef TRACING
	log(std::format("{}(r{})\n", __func__, index));
	flush();
#endif
	registers[index] = Value();
}

Value VM::getRegister(const uint8_t index)
{
#ifdef TRACING
	log(std::format("{}(r{}) : {}\n", __func__, index, registers[index]));
	flush();
#endif
	return registers[index];
}

size_t VM::getRegisterCount()
{
#ifdef TRACING
	log(std::format("{}() : {}\n", __func__, registers.size()));
	flush();
#endif
	return registers.size();
}

} // namespace Phasor