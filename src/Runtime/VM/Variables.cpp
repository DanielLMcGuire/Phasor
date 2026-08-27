#ifndef CMAKE_PCH
#include "VM.hpp"
#endif

#ifdef TRACING
#ifdef PHASOR_USES_BOOST
	#include <boost/assert/source_location.hpp>
	#define PHS_SRC_LOC() (std::format("{} @ {}:{}", BOOST_CURRENT_LOCATION.function_name(), BOOST_CURRENT_LOCATION.file_name(), BOOST_CURRENT_LOCATION.line()))
#else
	#define PHS_SRC_LOC() (std::format("VM::{}()", __func__))
#endif
#endif

namespace Phasor
{

size_t VM::addVariable(const Value &value)
{
#ifdef TRACING
	log(std::format("({})({:T})\n", PHS_SRC_LOC(), value));
	flush();
#endif
	variables.push_back(value);
	return variables.size() - 1;
}

void VM::freeVariable(const size_t index)
{
#ifdef TRACING
	log(std::format("({})({})\n", PHS_SRC_LOC(), index));
	flush();
#endif
	if (index < variables.size())
	{
		variables[index] = Value();
	}
}

void VM::freeVariableByName(const Phasor::string &name)
{
#ifdef TRACING
	log(std::format("({})(\"{}\")\n", PHS_SRC_LOC(), name));
	flush();
#endif
	if (m_bytecode == nullptr)
	{
		throw std::runtime_error("Error in freeVariable(): No bytecode loaded");
	}
	auto it = m_bytecode->variables.find(name);
	if (it == m_bytecode->variables.end())
	{
		throw std::runtime_error("Error in freeVariable(): Unknown variable \"" + name + "\"");
	}

	freeVariable(it->second);
}

void VM::setVariable(const size_t index, const Value &value)
{
#ifdef TRACING
	log(std::format("({})({}, {:T})\n", PHS_SRC_LOC(), index, value));
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
		log(std::format("({})({}) -> <invalid index>\n", PHS_SRC_LOC(), index));
		flush();
#endif
		throw std::runtime_error("Invalid variable index");
	}
#ifdef TRACING
	log(std::format("({})({}) -> {:T}\n", PHS_SRC_LOC(), index, variables[index]));
	flush();
#endif
	return variables[index];
}

size_t VM::getVariableCount()
{
#ifdef TRACING
	log(std::format("{}\n", PHS_SRC_LOC()));
	flush();
#endif
	return variables.size();
}

} // namespace Phasor