#include "VM.hpp"

namespace Phasor
{

void VM::setRegister(const uint8_t index, const Value &value)
{
	registers[index] = value;
}

void VM::freeRegister(const uint8_t index)
{
	registers[index] = Value(); // Reset to null
}

Value VM::getRegister(const uint8_t index)
{
	return registers[index];
}

size_t VM::getRegisterCount()
{
	return registers.size();
}

} // namespace Phasor