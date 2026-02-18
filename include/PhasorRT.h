#ifndef PHASOR_RT_H
#define PHASOR_RT_H

#include <stddef.h> // For size_t

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
	 * @brief Executes pre-compiled Phasor bytecode.
	 *
	 * @param embeddedBytecode An array of unsigned chars containing the Phasor bytecode.
	 * @param embeddedBytecodeSize The size of the bytecode array.
	 * @param moduleName The name of the module, used for error reporting.
	 * @param nativeFunctionsVector A pointer to a vector of pairs, where each pair contains a function name and a
	 * function pointer.
	 * @param argc The number of command-line arguments.
	 * @param argv An array of strings representing the command-line arguments.
	 */
	PHASOR_API void exec(const unsigned char embeddedBytecode[], size_t embeddedBytecodeSize, const char *moduleName,
	                     const void *nativeFunctionsVector, const int argc, const char **argv);

	/**
	 * @brief Executes a Phasor script using Just-In-Time (JIT) compilation.
	 *
	 * @param script A string containing the Phasor script to execute.
	 * @param moduleName The name of the module, used for error reporting.
	 * @param nativeFunctionsVector A pointer to a vector of pairs, where each pair contains a function name and a
	 * function pointer.
	 */
	PHASOR_API void jitExec(const char *script, const char *moduleName, const void *nativeFunctionsVector);

	/**
	 * @brief Executes a Phasor script within an existing VM instance.
	 *
	 * @param vm A pointer to the Phasor VM instance.
	 * @param script A string containing the Phasor script to execute.
	 */
	PHASOR_API void executeScript(void *vm, const char *script);

#ifdef __cplusplus
}
#endif

#endif // PHASOR_RT_H
