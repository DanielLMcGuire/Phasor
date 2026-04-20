// Copyright 2026 Daniel McGuire
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <stdlib.h>
#include <string.h>

#include "PhasorVM.hpp"
#include "../Value.hpp"

/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/// @brief Native function signature
using NativeFunction = std::function<Value(const std::vector<Value> &args, VM *vm)>;

/** 
 * @class StdLib
 * @brief Phasor Standard library
 *
 * Contains all the base standard library functions used for any language targetting the Phasor Runtime
 */
class StdLib
{
  public:
	inline static void registerFunctions(VM &vm)
	{
#ifdef TRACING
		vm.log(std::format("StdLib::{}(&VM@{:#x})\n", __func__, reinterpret_cast<std::uintptr_t>(&vm)));
		vm.flush();
#endif
		vm.registerNativeFunction("using", std_import);
#ifndef SANDBOXED
		vm.registerNativeFunction("assert", std_assert);
#endif
	}

	static char **argv; ///< Command line arguments
	static int    argc; ///< Number of command line arguments
	
	static void checkArgCount(const std::vector<Value> &args, size_t minimumArguments, const std::string &name,
	                          bool allowMoreArguments = false);
  private:
	static bool std_import(const std::vector<Value> &args, VM *vm);
#ifndef SANDBOXED
	static Value std_assert(const std::vector<Value> &args, VM *vm);
#endif
};

} // namespace Phasor
