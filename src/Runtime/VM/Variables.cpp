#include "VM.hpp"

namespace Phasor
{

size_t VM::addVariable(const Value &value)
{
	m_instance->variables.push_back(value);
	return m_instance->variables.size() - 1;
}

void VM::freeVariable(const size_t index)
{
	if (index < m_instance->variables.size())
	{
		m_instance->variables[index] = Value(); // Reset to null
	}
}

void VM::setVariable(const size_t index, const Value &value)
{
	if (index >= m_instance->variables.size())
	{
		throw std::runtime_error("Invalid variable index");
	}
	m_instance->variables[index] = value;
}

Value VM::getVariable(const size_t index)
{
	if (index >= m_instance->variables.size())
	{
		throw std::runtime_error("Invalid variable index");
	}
	return m_instance->variables[index];
}

size_t VM::getVariableCount()
{
	return m_instance->variables.size();
}

} // namespace Phasor