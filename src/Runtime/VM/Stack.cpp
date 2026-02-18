#include "VM.hpp"
#include <stdexcept>

namespace Phasor
{

void VM::push(const Value &value)
{
	stack.push_back(value);
}

Value VM::pop()
{
	if (stack.empty())
	{
		// Provide PC information to aid debugging
		std::string msg = "Stack underflow at pc=" + std::to_string(pc);
		throw std::runtime_error(msg);
	}
	Value value = stack.back();
	stack.pop_back();
	return value;
}

Value VM::peek()
{
	if (stack.empty())
	{
		std::string msg = "Stack is empty at pc=" + std::to_string(pc);
		throw std::runtime_error(msg);
	}
	return stack.back();
}

} // namespace Phasor