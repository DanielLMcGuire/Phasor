// Copyright 2025-2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://phasor.pages.dev/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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