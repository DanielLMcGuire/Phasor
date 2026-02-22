#include "VM.hpp"

namespace Phasor
{

void VM::setRegister(const uint8_t index, const Value &value)
{
	m_instance->activeFrame().registers[index] = value;
}

void VM::freeRegister(const uint8_t index)
{
	m_instance->activeFrame().registers[index] = Value();
}

Value VM::getRegister(const uint8_t index)
{
	return m_instance->activeFrame().registers[index];
}

size_t VM::getRegisterCount()
{
	return m_instance->activeFrame().registers.size();
}

} // namespace Phasor