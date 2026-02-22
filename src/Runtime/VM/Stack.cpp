#include "VM.hpp"
#include <stdexcept>

namespace Phasor
{

void VM::push(const Value &value)
{
	m_instance->activeFrame().stack.push_back(value);
}

Value VM::pop()
{
	if (m_instance->activeFrame().stack.empty())
	{
		// Provide PC information to aid debugging
		std::string msg = "Stack underflow at pc=" + std::to_string(m_instance->activeFrame().pc);
		throw std::runtime_error(msg);
	}
	Value value = m_instance->activeFrame().stack.back();
	m_instance->activeFrame().stack.pop_back();
	return value;
}

Value VM::peek()
{
	if (m_instance->activeFrame().stack.empty())
	{
		std::string msg = "Stack is empty at pc=" + std::to_string(m_instance->activeFrame().pc);
		throw std::runtime_error(msg);
	}
	return m_instance->activeFrame().stack.back();
}

} // namespace Phasor