#ifndef CMAKE
#include "VM.hpp"
#endif

namespace Phasor
{

size_t VM::addVariable(const Value &value)
{
#ifdef TRACING
	log(std::format("VM::{}({:T})\n", __func__, value));
	flush();
#endif
	variables.push_back(value);
	return variables.size() - 1;
}

void VM::freeVariable(const size_t index)
{
#ifdef TRACING
	log(std::format("VM::{}({})\n", __func__, index));
	flush();
#endif
	if (index < variables.size())
	{
		variables[index] = Value();
	}
}

void VM::setVariable(const size_t index, const Value &value)
{
#ifdef TRACING
	log(std::format("VM::{}({}, {:T})\n", __func__, index, value));
	flush();
#endif
	if (index >= variables.size())
	{
		throw std::runtime_error("Invalid variable index");
	}
	variables[index] = value;
}

Value VM::getVariable(const size_t index)
{
	if (index >= variables.size())
	{
#ifdef TRACING
		log(std::format("VM::{}({}) -> <invalid index>\n", __func__, index));
		flush();
#endif
		throw std::runtime_error("Invalid variable index");
	}
#ifdef TRACING
	log(std::format("VM::{}({}) -> {:T}\n", __func__, index, variables[index]));
	flush();
#endif
	return variables[index];
}

size_t VM::getVariableCount()
{
#ifdef TRACING
	log(std::format("VM::{}()\n", __func__));
	flush();
#endif
	return variables.size();
}

} // namespace Phasor