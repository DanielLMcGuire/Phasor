#include "VM.hpp"

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
		throw std::runtime_error("Stack underflow");
	}
	Value value = stack.back();
	stack.pop_back();
	return value;
}

Value VM::peek()
{
	if (stack.empty())
	{
		throw std::runtime_error("Stack is empty");
	}
	return stack.back();
}

} // namespace Phasor