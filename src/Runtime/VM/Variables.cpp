#ifndef CMAKE
#include "VM.hpp"
#endif

namespace Phasor
{

size_t VM::addVariable(const Value &value)
{
#ifdef TRACING
	log(std::format("{}({:T})\n", __func__, value));
	flush();
#endif
	variables.push_back(value);
	return variables.size() - 1;
}

void VM::freeVariable(const size_t index)
{
#ifdef TRACING
	log(std::format("{}({})\n", __func__, index));
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
	log(std::format("{}({}, {:T})\n", __func__, index, value));
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
#ifdef TRACING
	log(std::format("{}({})\n", __func__, index));
	flush();
#endif
	if (index >= variables.size())
	{
		throw std::runtime_error("Invalid variable index");
	}
	return variables[index];
}

size_t VM::getVariableCount()
{
#ifdef TRACING
	log(std::format("{}()\n", __func__));
	flush();
#endif
	return variables.size();
}

} // namespace Phasor