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

#include "../VM/VM.hpp"
#include <Value.hpp>

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
	static char **envp; ///< Environment variables
	
	static void checkArgCount(const std::vector<Value> &args, size_t minimumArguments, const std::string &name,
	                          bool allowMoreArguments = false);
  private:
	static bool std_import(const std::vector<Value> &args, VM *vm);
#ifndef SANDBOXED
	static Value std_assert(const std::vector<Value> &args, VM *vm);
#endif

#ifndef __EMSCRIPTEN__
#ifndef SANDBOXED
	static int dupenv(std::string &out, const char *name, char *const argp[]);
#endif

	static void registerMathFunctions(VM *vm);
	static void registerStringFunctions(VM *vm);
	static void registerTypeConvFunctions(VM *vm);
	static void registerFileFunctions(VM *vm);
	static void registerSysFunctions(VM *vm);
	static void registerIOFunctions(VM *vm);

#pragma region stdmath
	static double math_sqrt(const std::vector<Value> &args, VM *vm);  ///< Square root
	static double math_pow(const std::vector<Value> &args, VM *vm);   ///< Power
	static double math_abs(const std::vector<Value> &args, VM *vm);   ///< Absolute value
	static double math_floor(const std::vector<Value> &args, VM *vm); ///< Floor
	static double math_ceil(const std::vector<Value> &args, VM *vm);  ///< Ceiling
	static double math_round(const std::vector<Value> &args, VM *vm); ///< Round
	static Value math_min(const std::vector<Value> &args, VM *vm);   ///< Minimum
	static Value math_max(const std::vector<Value> &args, VM *vm);   ///< Maximum
	static double math_log(const std::vector<Value> &args, VM *vm);   ///< Natural logarithm
	static double math_exp(const std::vector<Value> &args, VM *vm);   ///< Exponential
	static double math_sin(const std::vector<Value> &args, VM *vm);   ///< Sine
	static double math_cos(const std::vector<Value> &args, VM *vm);   ///< Cosine
	static double math_tan(const std::vector<Value> &args, VM *vm);   ///< Tangent
#pragma endregion

#pragma region stdfile
#ifndef SANDBOXED
	static std::string file_absolute(const std::vector<Value> &args, VM *vm);     ///< Get full path to relative path
	static Value file_read(const std::vector<Value> &args, VM *vm);              ///< Read file
	static bool file_write(const std::vector<Value> &args, VM *vm);             ///< Write to file
	static bool file_exists(const std::vector<Value> &args, VM *vm);            ///< Check if file exists
	static std::string file_read_line(const std::vector<Value> &args, VM *vm);         ///< Read a line from file
	static bool file_write_line(const std::vector<Value> &args, VM *vm);        ///< Write a line to file
	static bool file_append(const std::vector<Value> &args, VM *vm);            ///< Append to file
	static bool file_delete(const std::vector<Value> &args, VM *vm);            ///< Delete file
	static bool file_rename(const std::vector<Value> &args, VM *vm);            ///< Rename file
	static Value file_current_directory(const std::vector<Value> &args, VM *vm); ///< Get/set working directory
	static bool file_copy(const std::vector<Value> &args, VM *vm);              ///< Copy file
	static bool file_move(const std::vector<Value> &args, VM *vm);              ///< Move file
	static bool file_property_edit(const std::vector<Value> &args, VM *vm);
	static int64_t file_property_get(const std::vector<Value> &args, VM *vm);
	static bool file_create(const std::vector<Value> &args, VM *vm);
	static Value file_read_directory(const std::vector<Value> &args, VM *vm);
	static Value file_statistics(const std::vector<Value> &args, VM *vm);
	static bool file_create_directory(const std::vector<Value> &args, VM *vm);
	static bool file_remove_directory(const std::vector<Value> &args, VM *vm);
#pragma endregion


#pragma region stdsys
	static std::string sys_env(const std::vector<Value> &args, VM *vm);            ///< Get the current environment variables
	static Value sys_argv(const std::vector<Value> &args, VM *vm);           ///< Get the current command line arguments
	static int64_t sys_get_free_memory(const std::vector<Value> &args, VM *vm); ///< Get current free memory
	static int64_t sys_argc(const std::vector<Value> &args, VM *vm); ///< Get the current number of command line arguments
	static Value sys_wait_for_input(const std::vector<Value> &args, VM *vm);  ///< Wait for input
	static Value sys_shell(const std::vector<Value> &args, VM *vm);            ///< Run a shell command
	static int64_t sys_fork(const std::vector<Value> &args, VM *vm);           ///< Run a native program
	static int64_t sys_fork_detached(const std::vector<Value> &args, VM *vm); ///< Run a native program detached
	static Value sys_crash(const std::vector<Value> &args, VM *vm);    ///< Crash the VM / Program
	static Value sys_reset(const std::vector<Value> &args, VM *vm);    ///< Reset the VM
	static int64_t sys_pid(const std::vector<Value> &args, VM *vm);      ///< Get the current process ID
	static std::string sys_os(const std::vector<Value> &args, VM *vm);             ///< Get the current OS
	static Value sys_isatty(const std::vector<Value> &args, VM *vm);
#endif
	static double sys_time(const std::vector<Value> &args, VM *vm);           ///< Current time
	static Value sys_time_formatted(const std::vector<Value> &args, VM *vm); ///< Current time formatted
	static Value sys_sleep(const std::vector<Value> &args, VM *vm);          ///< Sleep for a specified amount of time
	static Value sys_shutdown(const std::vector<Value> &args, VM *vm); ///< Shutdown the VM
#pragma endregion

#pragma region stdtype
	static int64_t to_int(const std::vector<Value> &args, VM *vm);    ///< Convert to integer
	static double to_float(const std::vector<Value> &args, VM *vm);  ///< Convert to float
	static std::string to_string(const std::vector<Value> &args, VM *vm); ///< Convert to string
	static bool to_bool(const std::vector<Value> &args, VM *vm);   ///< Convert to boolean
#pragma endregion

#pragma region stdstr
	static int64_t str_find(const std::vector<Value> &args, VM *vm);        ///< Find string in string
	static int64_t str_len(const std::vector<Value> &args, VM *vm);         ///< Get string length
	static Value str_char_at(const std::vector<Value> &args, VM *vm);     ///< Get character at index
	static Value str_substr(const std::vector<Value> &args, VM *vm);      ///< Get substring
	static std::string str_concat(const std::vector<Value> &args, VM *vm);      ///< Concatenate strings
	static std::string str_upper(const std::vector<Value> &args, VM *vm);       ///< Convert to uppercase
	static std::string str_lower(const std::vector<Value> &args, VM *vm);       ///< Convert to lowercase
	static Value str_starts_with(const std::vector<Value> &args, VM *vm); ///< Check if string starts with
	static Value str_ends_with(const std::vector<Value> &args, VM *vm);   ///< Check if string ends with
	// StringBuilder functions
	static int64_t sb_new(const std::vector<Value> &args, VM *vm);       ///< Create new string builder
	static Value sb_append(const std::vector<Value> &args, VM *vm);    ///< Append to string builder
	static std::string sb_to_string(const std::vector<Value> &args, VM *vm); ///< Convert string builder to string
	static Value sb_clear(const std::vector<Value> &args, VM *vm);     ///< Clear string builder
	static std::string sb_free(const std::vector<Value> &args, VM *vm);      ///< Free string builder
#pragma endregion

#pragma region stdio
	static std::string io_c_format(const std::vector<Value> &args, VM *vm); ///< Format string
#ifndef SANDBOXED
	static Value io_clear(const std::vector<Value> &args, VM *vm);    ///< Clear the console
#endif
	static Value io_prints(const std::vector<Value> &args, VM *vm);   ///< Print string without newline
	static Value io_printf(const std::vector<Value> &args, VM *vm);   ///< Print formatted string
	static Value io_puts(const std::vector<Value> &args, VM *vm);     ///< Print string with newline
	static Value io_putf(const std::vector<Value> &args, VM *vm);     ///< Print formatted string with newline
#ifndef SANDBOXED
	static Value io_gets(const std::vector<Value> &args, VM *vm);     ///< Get string
#endif
	static Value io_putf_error(const std::vector<Value> &args,
	                           VM                       *vm); ///< Print formatted string with newline to error output
	static Value io_puts_error(const std::vector<Value> &args, VM *vm); ///< Print string with newline to error output
#endif
#pragma endregion
};

} // namespace Phasor