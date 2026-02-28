#pragma once
#include "../VM/VM.hpp"
#include <Value.hpp>
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
/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/// @brief Native function signature
using NativeFunction = std::function<Value(const std::vector<Value> &args, VM *vm)>;

/// @class StdLib
/// @brief Standard library
/// Contains all the standard library functions
class StdLib
{
  public:
	static void registerFunctions(VM &vm);

	static char **argv; ///< Command line arguments
	static int    argc; ///< Number of command line arguments
	static char **envp; ///< Environment variables

	static void checkArgCount(const std::vector<Value> &args, size_t minimumArguments, const std::string &name,
	                          bool allowMoreArguments = false);

  private:
	static Value std_import(const std::vector<Value> &args, VM *vm);

	static int dupenv(std::string &out, const char *name, char *const argp[]);

	static Value registerMathFunctions(const std::vector<Value> &args, VM *vm);
	static Value registerStringFunctions(const std::vector<Value> &args, VM *vm);
	static Value registerTypeConvFunctions(const std::vector<Value> &args, VM *vm);
	static Value registerFileFunctions(const std::vector<Value> &args, VM *vm);
	static Value registerSysFunctions(const std::vector<Value> &args, VM *vm);
	static Value registerIOFunctions(const std::vector<Value> &args, VM *vm);

	// Math functions
	static Value math_sqrt(const std::vector<Value> &args, VM *vm);  ///< Square root
	static Value math_pow(const std::vector<Value> &args, VM *vm);   ///< Power
	static Value math_abs(const std::vector<Value> &args, VM *vm);   ///< Absolute value
	static Value math_floor(const std::vector<Value> &args, VM *vm); ///< Floor
	static Value math_ceil(const std::vector<Value> &args, VM *vm);  ///< Ceiling
	static Value math_round(const std::vector<Value> &args, VM *vm); ///< Round
	static Value math_min(const std::vector<Value> &args, VM *vm);   ///< Minimum
	static Value math_max(const std::vector<Value> &args, VM *vm);   ///< Maximum
	static Value math_log(const std::vector<Value> &args, VM *vm);   ///< Natural logarithm
	static Value math_exp(const std::vector<Value> &args, VM *vm);   ///< Exponential
	static Value math_sin(const std::vector<Value> &args, VM *vm);   ///< Sine
	static Value math_cos(const std::vector<Value> &args, VM *vm);   ///< Cosine
	static Value math_tan(const std::vector<Value> &args, VM *vm);   ///< Tangent

	// File IO
	static Value file_absolute(const std::vector<Value> &args, VM *vm);     ///< Get full path to relative path
	static Value file_read(const std::vector<Value> &args, VM *vm);              ///< Read file
	static Value file_write(const std::vector<Value> &args, VM *vm);             ///< Write to file
	static Value file_exists(const std::vector<Value> &args, VM *vm);            ///< Check if file exists
	static Value file_read_line(const std::vector<Value> &args, VM *vm);         ///< Read a line from file
	static Value file_write_line(const std::vector<Value> &args, VM *vm);        ///< Write a line to file
	static Value file_append(const std::vector<Value> &args, VM *vm);            ///< Append to file
	static Value file_delete(const std::vector<Value> &args, VM *vm);            ///< Delete file
	static Value file_rename(const std::vector<Value> &args, VM *vm);            ///< Rename file
	static Value file_current_directory(const std::vector<Value> &args, VM *vm); ///< Get/set working directory
	static Value file_copy(const std::vector<Value> &args, VM *vm);              ///< Copy file
	static Value file_move(const std::vector<Value> &args, VM *vm);              ///< Move file
	static Value file_property_edit(const std::vector<Value> &args, VM *vm);
	static Value file_property_get(const std::vector<Value> &args, VM *vm);
	static Value file_create(const std::vector<Value> &args, VM *vm);
	static Value file_read_directory(const std::vector<Value> &args, VM *vm);
	static Value file_statistics(const std::vector<Value> &args, VM *vm);
	static Value file_create_directory(const std::vector<Value> &args, VM *vm);
	static Value file_remove_directory(const std::vector<Value> &args, VM *vm);

	// System (meaning VM/CRT more than actual system)
	static Value sys_time(const std::vector<Value> &args, VM *vm);           ///< Current time
	static Value sys_time_formatted(const std::vector<Value> &args, VM *vm); ///< Current time formatted
	static Value sys_sleep(const std::vector<Value> &args, VM *vm);          ///< Sleep for a specified amount of time
	static Value sys_os(const std::vector<Value> &args, VM *vm);             ///< Get the current OS
	static Value sys_env(const std::vector<Value> &args, VM *vm);            ///< Get the current environment variables
	static Value sys_argv(const std::vector<Value> &args, VM *vm);           ///< Get the current command line arguments
	static Value system_get_free_memory(const std::vector<Value> &args, VM *vm); ///< Get current free memory
	static Value sys_argc(const std::vector<Value> &args, VM *vm); ///< Get the current number of command line arguments
	static Value sys_wait_for_input(const std::vector<Value> &args, VM *vm);  ///< Wait for input
	static Value sys_shell(const std::vector<Value> &args, VM *vm);            ///< Run a shell command
	static Value sys_fork(const std::vector<Value> &args, VM *vm);           ///< Run a native program
	static Value sys_fork_detached(const std::vector<Value> &args, VM *vm); ///< Run a native program detached
	static Value sys_crash(const std::vector<Value> &args, VM *vm);    ///< Crash the VM / Program
	static Value sys_reset(const std::vector<Value> &args, VM *vm);    ///< Reset the VM
	static Value sys_shutdown(const std::vector<Value> &args, VM *vm); ///< Shutdown the VM
	static Value sys_pid(const std::vector<Value> &args, VM *vm);      ///< Get the current process ID

	// Type conversion functions
	static Value to_int(const std::vector<Value> &args, VM *vm);    ///< Convert to integer
	static Value to_float(const std::vector<Value> &args, VM *vm);  ///< Convert to float
	static Value to_string(const std::vector<Value> &args, VM *vm); ///< Convert to string
	static Value to_bool(const std::vector<Value> &args, VM *vm);   ///< Convert to boolean

	// String functions
	static Value str_find(const std::vector<Value> &args, VM *vm);        ///< Find string in string
	static Value str_len(const std::vector<Value> &args, VM *vm);         ///< Get string length
	static Value str_char_at(const std::vector<Value> &args, VM *vm);     ///< Get character at index
	static Value str_substr(const std::vector<Value> &args, VM *vm);      ///< Get substring
	static Value str_concat(const std::vector<Value> &args, VM *vm);      ///< Concatenate strings
	static Value str_upper(const std::vector<Value> &args, VM *vm);       ///< Convert to uppercase
	static Value str_lower(const std::vector<Value> &args, VM *vm);       ///< Convert to lowercase
	static Value str_starts_with(const std::vector<Value> &args, VM *vm); ///< Check if string starts with
	static Value str_ends_with(const std::vector<Value> &args, VM *vm);   ///< Check if string ends with
	// StringBuilder functions
	static Value sb_new(const std::vector<Value> &args, VM *vm);       ///< Create new string builder
	static Value sb_append(const std::vector<Value> &args, VM *vm);    ///< Append to string builder
	static Value sb_to_string(const std::vector<Value> &args, VM *vm); ///< Convert string builder to string
	static Value sb_clear(const std::vector<Value> &args, VM *vm);     ///< Clear string builder
	static Value sb_free(const std::vector<Value> &args, VM *vm);      ///< Free string builder

	// IO
	static Value io_c_format(const std::vector<Value> &args, VM *vm); ///< Format string
	static Value io_clear(const std::vector<Value> &args, VM *vm);    ///< Clear the console
	static Value io_prints(const std::vector<Value> &args, VM *vm);   ///< Print string without newline
	static Value io_printf(const std::vector<Value> &args, VM *vm);   ///< Print formatted string
	static Value io_puts(const std::vector<Value> &args, VM *vm);     ///< Print string with newline
	static Value io_putf(const std::vector<Value> &args, VM *vm);     ///< Print formatted string with newline
	static Value io_gets(const std::vector<Value> &args, VM *vm);     ///< Get string
	static Value io_putf_error(const std::vector<Value> &args,
	                           VM                       *vm); ///< Print formatted string with newline to error output
	static Value io_puts_error(const std::vector<Value> &args, VM *vm); ///< Print string with newline to error output
};

} // namespace Phasor