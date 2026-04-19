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
