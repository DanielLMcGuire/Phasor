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

#ifdef TRACING_STACK
#ifdef PHASOR_USES_BOOST
	#include <boost/assert/source_location.hpp>
	#define PHS_SRC_LOC() (std::format("{} @ {}:{}", BOOST_CURRENT_LOCATION.function_name(), BOOST_CURRENT_LOCATION.file_name(), BOOST_CURRENT_LOCATION.line()))
#else
	#define PHS_SRC_LOC() (std::format("VM::{}()", __func__))
#endif
#endif

namespace Phasor
{

void VM::push(const Value &value)
{
#ifdef TRACING_STACK
	log(std::format("({})({:T})\n", PHS_SRC_LOC(), value));
	flush();
#endif
	stack.push_back(value);
}

Value VM::pop()
{
	if (stack.empty())
	{
#ifdef TRACING_STACK
		log(std::format("({}) -> <empty stack>\n", PHS_SRC_LOC()));
		flush();
#endif
		Phasor::string msg = "Stack underflow at pc=" + std::to_string(pc);
		throw std::runtime_error(msg.str());
		return phsnull;
	}
#ifdef TRACING_STACK
	log(std::format("({}) -> {:T}\n", PHS_SRC_LOC(), stack.back()));
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
#ifdef TRACING_STACK
		log(std::format("({}) -> <empty stack>\n", PHS_SRC_LOC()));
		flush();
#endif
		Phasor::string msg = "Stack is empty at pc=" + std::to_string(pc);
		throw std::runtime_error(msg.str());
		return phsnull;
	}
#ifdef TRACING_STACK
	log(std::format("({}) -> {:T}\n", PHS_SRC_LOC(), stack.back()));
	flush();
#endif
	return stack.back();
}

} // namespace Phasor