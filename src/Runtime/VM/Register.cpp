#ifndef CMAKE
#include "VM.hpp"
#endif

namespace Phasor
{

void VM::setRegister(const uint8_t index, const Value &value)
{
#ifdef _DEBUG
	log(std::format("{}('{}', {}: {})\n", __func__, index, value, Value::typeToString(value.getType())));
	flush();
#endif
	registers[index] = value;
}

void VM::freeRegister(const uint8_t index)
{
#ifdef _DEBUG
	log(std::format("{}({})\n", __func__, index));
	flush();
#endif
	registers[index] = Value();
}

Value VM::getRegister(const uint8_t index)
{
#ifdef _DEBUG
	log(std::format("{}({})\n", __func__, index));
	flush();
#endif
	return registers[index];
}

size_t VM::getRegisterCount()
{
#ifdef _DEBUG
	log(std::format("{}()\n", __func__));
	flush();
#endif
	return registers.size();
}

} // namespace Phasor