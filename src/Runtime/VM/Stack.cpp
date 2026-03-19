#ifndef CMAKE
#include "VM.hpp"
#endif

namespace Phasor
{

void VM::push(const Value &value)
{
#ifdef TRACING
	log(std::format("{}({:T})\n", __func__, value));
	flush();
#endif
	stack.push_back(value);
}

Value VM::pop()
{
	if (stack.empty())
	{
#ifdef TRACING
		log(std::format("{}() : <empty stack>\n", __func__));
		flush();
#endif
		std::string msg = "Stack underflow at pc=" + std::to_string(pc);
		throw std::runtime_error(msg);
		return Value();
	}
#ifdef TRACING
	log(std::format("{}() : {:T}\n", __func__, stack.back()));
	flush();
#endif
	Value value = stack.back();
	stack.pop_back();
	return value;
}

Value VM::peek()
{
	if (stack.empty())
	{
#ifdef TRACING
		log(std::format("{}() : <empty stack>\n", __func__));
		flush();
#endif
		std::string msg = "Stack is empty at pc=" + std::to_string(pc);
		throw std::runtime_error(msg);
		return Value();
	}
#ifdef TRACING
	log(std::format("{}() : {:T}\n", __func__, stack.back()));
	flush();
#endif
	return stack.back();
}

} // namespace Phasor