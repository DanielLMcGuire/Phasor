#pragma once

#include <functional>
#include "Value.hpp"

#include <filesystem>
#include <vector>
#include <string>


#if defined(_WIN32)
#include <windows.h>
#else 
#include <dlfcn.h>
#endif

#include "PhasorFFI.h"

namespace Phasor {
    class VM;
}

using FFIFunction = void (*)(const PhasorAPI *api, PhasorVM *vm);
/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/**
 * @brief Represents a loaded plugin.
 *
 * Stores the library handle, the plugin's file path,
 * the initialization function, and an optional shutdown function.
 */
struct Plugin
{
#if defined(_WIN32)
	HMODULE handle; ///< Windows handle for the loaded library
#else
	void *handle; ///< POSIX handle for the loaded library
#endif
	std::string           path;     ///< Path to the plugin file
	FFIFunction           init;     ///< Plugin initialization function
	std::function<void()> shutdown; ///< Optional shutdown callback
};

/**
 * @brief The "trampoline" that wraps a C function from a plugin.
 */
Phasor::Value c_native_func_wrapper(PhasorNativeFunction c_func, Phasor::VM *vm,
                                    const std::vector<Phasor::Value> &args);

/**
 * @brief The concrete implementation of the `PhasorRegisterFunction` API call.
 *
 * This function is passed to the plugin. When the plugin calls it, this function
 * creates a C++ lambda that wraps the plugin's C function pointer and registers
 * that lambda with the Phasor VM.
 */
void register_native_c_func(PhasorVM *vm, const char *name, PhasorNativeFunction func);

/**
 * @brief Manages loading, registering, and unloading native FFI plugins.
 *
 * This class scans a folder for plugins, loads them, registers their
 * functions with the VM, and handles cleanup when destroyed.
 */
class FFI
{
  public:
	/**
	 * @brief Constructs the FFI manager and loads plugins.
	 * @param pluginFolder Path to the folder containing plugins.
	 * @param vm Pointer to the Phasor VM instance to register plugin functions with.
	 */
	explicit FFI(const std::filesystem::path &pluginFolder, VM *vm);

	/**
	 * @brief Destructor. Unloads all loaded plugins.
	 */
	~FFI();

	/**
	 * @brief Adds a single plugin from the specified path.
	 * @param pluginPath Path to the plugin file.
	 * @return True if the plugin was added successfully, false otherwise.
	 */
	bool addPlugin(const std::filesystem::path &pluginPath);
};

} // namespace Phasor
