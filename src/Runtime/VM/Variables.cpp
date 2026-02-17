#include "VM.hpp"

namespace Phasor
{

size_t VM::addVariable(const Value &value)
{
	variables.push_back(value);
	return variables.size() - 1;
}

void VM::freeVariable(const size_t index)
{
	if (index < variables.size())
	{
		variables[index] = Value(); // Reset to null
	}
}

void VM::setVariable(const size_t index, const Value &value)
{
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
		throw std::runtime_error("Invalid variable index");
	}
	return variables[index];
}

size_t VM::getVariableCount()
{
	return variables.size();
}

} // namespace Phasor