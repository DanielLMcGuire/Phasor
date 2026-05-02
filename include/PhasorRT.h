// Copyright 2026 Daniel McGuire
// Licensed under the Apache License (with LLVM-Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// https://llvm.org/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// README
//
// This is the main header file for the Phasor Runtime C ABI.
// The goal is to keep this ABI as stable as possible, see the details below, or at
// https://phasor-docs.pages.dev/man?f=phasorrt.3
// man phasorrt

#ifndef PHASOR_RT_H
#define PHASOR_RT_H

#ifdef __cplusplus
#include <cstddef>
#else
#include <stdbool.h>
#include <stddef.h>
#endif

#ifdef _WIN32
#ifdef PHASOR_EXPORTS
#define PHASOR_API __declspec(dllexport)
#else
#define PHASOR_API __declspec(dllimport)
#endif
#elif defined(__GNUC__) || defined(__clang__)
#ifdef PHASOR_EXPORTS
#define PHASOR_API __attribute__((visibility("default")))
#else
#define PHASOR_API
#endif
#else
#define PHASOR_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif
	/**
	 * @brief  Get the version string for Phasor VM.
	 * @return The version string.
	 */
	PHASOR_API const char *getVersion();

	/**
	 * @brief Executes pre-compiled Phasor bytecode.
	 *
	 * @param state                A pointer to an state to execute the script within. If null, new state will be
	 * created and managed for you.
	 * @param bytecode             An array of unsigned chars containing the Phasor bytecode.
	 * @param bytecodeSize         The size of the bytecode array.
	 * @param moduleName           The name of the module, used for error reporting.
	 * @param argc                 Argument count.
	 * @param argv                 Argument vector.
	 * @return                     The exit code of the program given from script (-1 might be an unhandled exception in
	 * VM).
	 */
	PHASOR_API int exec(void *state, const unsigned char *bytecode, size_t bytecodeSize, const char *moduleName,
	                    int argc, const char **argv);

	/**
	 * @brief Executes a function from pre-compiled Phasor bytecode, and casts return to an integer.
	 *
	 * @param state                A pointer to an state to execute the script within. If null, new state will be
	 * created and managed for you.
	 * @param bytecode             An array of unsigned chars containing the Phasor bytecode.
	 * @param bytecodeSize         The size of the bytecode array.
	 * @param moduleName           The name of the module, used for error reporting.
	 * @param argc                 Argument count.
	 * @param argv                 Argument vector.
	 * @param functionName         The name of the function to execute.
	 * @return                     The return from the function call (-1 might be an unhandled exception in VM).
	 */
	PHASOR_API int execFuncInt(void *state, const unsigned char *bytecode, size_t bytecodeSize, const char *moduleName,
	                           int argc, const char **argv, const char *functionName);

	/**
	 * @brief Executes a function from pre-compiled Phasor bytecode, and casts return to an string.
	 *
	 * @param state                A pointer to an state to execute the script within. If null, new state will be
	 * created and managed for you.
	 * @param bytecode             An array of unsigned chars containing the Phasor bytecode.
	 * @param bytecodeSize         The size of the bytecode array.
	 * @param moduleName           The name of the module, used for error reporting.
	 * @param argc                 Argument count.
	 * @param argv                 Argument vector.
	 * @param functionName         The name of the function to execute.
	 * @return                     The return from the function call, nullptr on error. This memory is cleared by the
	 * C++ runtime when you unload the DLL, and is overwritten after recalling this function. This DOES NOT use two pass
	 * for perf reasons. Copy the string to safe memory, or you will lose the data.
	 */
	PHASOR_API const char *execFuncString(void *state, const unsigned char *bytecode, size_t bytecodeSize,
	                                      const char *moduleName, int argc, const char **argv,
	                                      const char *functionName);

	/**
	 * @brief Executes a Phasor Programming Language script.
	 *
	 * @param state      A pointer to an state to execute the script within. If null, new state will be created and
	 * managed for you.
	 * @param script     A string containing the Phasor source to compile and execute.
	 * @param moduleName The name of the module, used for error reporting.
	 * @param modulePath A required path to the parent folder for the script, used for resolving compile time imports.
	 * @param verbose    Prints AST to stdout.
	 * @return           The exit code of the program given from script (-1 might be an unhandled exception in VM).
	 */
	PHASOR_API int evaluatePHS(void *state, const char *script, const char *moduleName, const char *modulePath,
	                           bool verbose);

	/**
	 * @brief Executes a Pulsar Scripting Language script.
	 *
	 * @param state      A pointer to an state to execute the script within. If null, new state will be created and
	 * managed for you.
	 * @param script     A string containing the Pulsar source to compile and execute.
	 * @param moduleName The name of the module, used for error reporting.
	 * @return           The exit code of the program given from script (-1 might be an unhandled exception in VM).
	 */
	PHASOR_API int evaluatePUL(void *state, const char *script, const char *moduleName);

	/**
	 * @brief Compiles a Phasor Programming Language script into Phasor VM bytecode.
	 *
	 * @param[in] script      A string containing the Phasor source to compile.
	 * @param[in] moduleName  The name of the module, used for error reporting.
	 * @param[in] modulePath  An required path to the parent folder for the script, used for resolving compile time
	 * imports.
	 * @param[out] buffer     A pointer to a buffer where the compiled bytecode will be written. If null, the function
	 * will only calculate the required buffer size and return it via `outSize`.
	 * @param[in] bufferSize The size of the provided buffer. This is ignored if `buffer` is null.
	 * @param[out] outSize   If `buffer` is null, this will be set to the required buffer size to hold the compiled
	 * bytecode. If `buffer` is not null, this will be set to the actual size of the compiled bytecode.
	 * @returns              if buffer is null, returns true if compilation succeeded. if buffer is not null, returns
	 * true if compilation succeeded and buffer was valid and large enough to hold the compiled bytecode.
	 *
	 * if bufferSize < data.size(), buffer will not be modified, however outSize will be set.
	 */
	PHASOR_API bool compilePHS(const char *script, const char *moduleName, const char *modulePath,
	                           unsigned char *buffer, size_t bufferSize, size_t *outSize);

	/**
	 * @brief Compiles a Pulsar Scripting Language script into Phasor VM bytecode.
	 *
	 * @param[in] script     A string containing the Pulsar source to compile.
	 * @param[in] moduleName The name of the module, used for error reporting.
	 * @param[out] buffer    A pointer to a buffer where the compiled bytecode will be written. If null, the function
	 * will only calculate the required buffer size and return it via `outSize`.
	 * @param[in] bufferSize The size of the provided buffer. This is ignored if `buffer` is null.
	 * @param[out] outSize   If `buffer` is null, this will be set to the required buffer size to hold the compiled
	 * bytecode. If `buffer` is not null, this will be set to the actual size of the compiled bytecode.
	 * @returns              if buffer is null, returns true if compilation succeeded. if buffer is not null, returns
	 * true if compilation succeeded and buffer was valid and large enough to hold the compiled bytecode.
	 *
	 * if bufferSize < data.size(), buffer will not be modified, however outSize will be set.
	 */
	PHASOR_API bool compilePUL(const char *script, const char *moduleName, unsigned char *buffer, size_t bufferSize,
	                           size_t *outSize);

	/**
	 * @brief Creates a new state instance.
	 *
	 * @return Pointer to the newly created state.
	 *
	 * A state is a `Phasor::VM` object, refered to as `State` from a high level to avoid bad programming.
	 * Dependence on the internal structure of the VM class is not *officially* supported and will not work across
	compilers.
	 * This version of the C API is designed to work across compilers and outlive minor, potentially even major versions
	of the VM.
	 *
	@code
	void *state = createState();
	initStdLib(state);

	const char *script = "using(\\\"stdio\\\"); puts(\\\"Hello, World!\\");";

	size_t bytecodeSize = 0;
	compilePHS(script, "example", "./", NULL, 0, &bytecodeSize);

	unsigned char *bytecode = (unsigned char *)malloc(bytecodeSize);
	compilePHS(script, "example", "./", bytecode, bytecodeSize, &bytecodeSize);

	const char *argv[] = { "program", "script", "arg1", "arg2" };
	int result = exec(state, bytecode, bytecodeSize, "example", 4, argv);

	free(bytecode);
	freeState(state);
	@endcode
	 */
	PHASOR_API void *createState();

	/**
	 * @brief Register standard library to state instance.
	 *
	 * @param state Pointer to state.
	 *
	 * Registers standard library functions to a State instance.
	 */
	PHASOR_API void initStdLib(void *state);

	/**
	 * @brief Frees an existing state instance.
	 *
	 * @param state Pointer to state to free.
	 * @return false if the data was seemingly invalid or null, true otherwise.
	 *
	 * This function like any low level function could be potentially unsafe if given the wrong thing.
	 * Support will not be accepted in these scenarios. Do not call many times on the same pointer.
	 */
	PHASOR_API bool freeState(void *state);

	/**
	 * @brief Resets the state.
	 *
	 * @param state          Pointer to the state to reset.
	 * @param resetFunctions Reset functions if set to true.
	 * @param resetVariables Reset variables if set to true.
	 *
	 * This will always clear the stack, reset the PC, and clear bytecode.
	 */
	PHASOR_API bool resetState(void *state, bool resetFunctions, bool resetVariables);

#ifdef __cplusplus
}
#endif

#endif // PHASOR_RT_H
