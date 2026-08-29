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
#include <utility>

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

void Phasor::VM::registerNativeFunction(const Phasor::string &name, NativeFunction fn)
{
#ifdef TRACING
	log(std::format("({})(\"{}\")\n", PHS_SRC_LOC(), name));
	flush();
#endif
	nativeFunctions[name] = std::move(fn);
}