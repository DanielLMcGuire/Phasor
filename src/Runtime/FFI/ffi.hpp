#pragma once

#include <functional>
#include <filesystem>

#ifndef CMAKE_PCH
#include <Value.hpp>
#endif
#include <vector>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <PhasorFFI.h>

namespace Phasor
{
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
	 * @param pluginFolders Vector of paths to folders containing plugins.
	 * @param vm Pointer to the Phasor VM instance to register plugin functions with.
	 */
	explicit FFI(const std::vector<std::filesystem::path> &pluginFolders, VM *vm);

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

	/**
	 * @brief Retrieves FFI instance tied to a specific VM.
	 */
	static FFI* getFromVM(VM* vm);

	/**
	 * @brief Registers a function pointer to be called upon FFI unloading.
	 */
	void registerExitCall(void (*func)());

	/**
	 * @brief Registers a dynamically allocated pointer to be freed upon FFI unloading.
	 */
	void registerExitFree(void* ptr);

	/**
	 * @brief Safely returns cached version string without allocating a dangling pointer.
	 */
	const char* getVersionString() const { return cachedVersion_.c_str(); }

  private:
	/**
	 * @brief Native function to load a plugin at runtime.
	 */
	bool native_add_plugin(const std::vector<Value> &args, VM *vm);

	/**
	 * @brief Loads a single plugin from a library file.
	 * @param library Path to the shared library file.
	 * @param vm Pointer to the VM to register plugin functions with.
	 * @return True if the plugin loaded successfully, false otherwise.
	 */
	bool loadPlugin(const std::filesystem::path &library, VM *vm);

	/**
	 * @brief Scans multiple folders for plugin libraries.
	 * @param folders Vector of folder paths to scan.
	 * @return A vector of plugin file paths.
	 */
	std::vector<std::string> scanPlugins(const std::vector<std::filesystem::path> &folders);


	/**
	 * @brief Unloads all currently loaded plugins and clears internal state.
	 */
	void unloadAll();

	std::vector<Plugin>   plugins_;      ///< Loaded plugins
	std::vector<std::filesystem::path> pluginFolders_; ///< Plugin search folder
	VM                   *vm_;           ///< Pointer to the Phasor VM

	std::string           cachedVersion_;///< Cached version string tied to the VM
	std::vector<void (*)()> exitCalls_;  ///< Callbacks for onExit
	std::vector<void*>      exitFrees_;  ///< Pointers mapped for onExit freeing
};

} // namespace Phasor

#ifdef __cplusplus
extern "C" {
#endif

void api_log(PhasorVM* vm, PhasorValue msg);
void api_logerr(PhasorVM* vm, PhasorValue msg);
void api_flush(PhasorVM* vm);
void api_flusherr(PhasorVM* vm);
const char* api_getVersion(PhasorVM* vm);
bool api_loadPlugin(PhasorVM* vm, const char* libPath);
void api_onExitCall(PhasorVM* vm, void (*func)(void));
void api_onExitFree(PhasorVM* vm, void* ptr);
void* api_malloc(size_t size);
void* api_calloc(size_t num, size_t size);
void* api_realloc(void* ptr, size_t size);
void api_free(void* ptr);

#ifdef __cplusplus
}
#endif