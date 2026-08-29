// Copyright 2026 Daniel McGuire
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
#include <phsint.hpp>

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

void VM::setRegister(const u8 index, const Value &value)
{
#ifdef TRACING
	log(std::format("({})r{}, {:T})\n", PHS_SRC_LOC(), index, value));
	flush();
#endif
	registers[index] = value;
}

void VM::freeRegister(const u8 index)
{
#ifdef TRACING
	log(std::format("({})(r{})\n", PHS_SRC_LOC(), index));
	flush();
#endif
	registers[index] = Value();
}

Value VM::getRegister(const u8 index)
{
#ifdef TRACING
	log(std::format("({})(r{}) -> {:T}\n", PHS_SRC_LOC(), index, registers[index]));
	flush();
#endif
	return registers[index];
}

size_t VM::getRegisterCount()
{
#ifdef TRACING
	log(std::format("{} -> {}\n", PHS_SRC_LOC(), registers.size()));
	flush();
#endif
	return registers.size();
}

} // namespace Phasor