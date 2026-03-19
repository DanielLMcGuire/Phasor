#ifndef CMAKE
#include "VM.hpp"
#endif

namespace Phasor
{

void VM::push(const Value &value)
{
#ifdef _DEBUG
	log(std::format("{}('{}': {})\n", __func__, escapeString(value.toString()), Value::typeToString(value.getType())));
	flush();
#endif
	stack.push_back(value);
}

Value VM::pop()
{
#ifdef _DEBUG
	log(std::format("{}()\n", __func__));
	flush();
#endif
	if (stack.empty())
	{
		std::string msg = "Stack underflow at pc=" + std::to_string(pc);
		throw std::runtime_error(msg);
		return Value();
	}
	Value value = stack.back();
	stack.pop_back();
	return value;
}

Value VM::peek()
{
#ifdef _DEBUG
	log(std::format("{}()\n", __func__));
	flush();
#endif
	if (stack.empty())
	{
		std::string msg = "Stack is empty at pc=" + std::to_string(pc);
		throw std::runtime_error(msg);
		return Value();
	}
	return stack.back();
}

} // namespace Phasor