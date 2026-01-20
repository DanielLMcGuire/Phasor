#pragma once

#include <functional>
#include "../Value.hpp"
#include "../VM/VM.hpp"

#include <vector>
#include <string>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <PhasorFFI.hpp>

typedef void (*FFIFunction)(const PhasorAPI *api, PhasorVM *vm);

namespace Phasor
{

/**
 * @brief Represents a loaded plugin.
 *
 * Stores the platform-specific library handle, the plugin's file path,
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
	explicit FFI(const std::string &pluginFolder, VM *vm);

	/**
	 * @brief Destructor. Unloads all loaded plugins.
	 */
	~FFI();

  private:
	/**
	 * @brief Loads a single plugin from a library file.
	 * @param library Path to the shared library file.
	 * @param vm Pointer to the VM to register plugin functions with.
	 * @return True if the plugin loaded successfully, false otherwise.
	 */
	bool loadPlugin(const std::filesystem::path &library, VM *vm);

	/**
	 * @brief Scans a folder for plugin libraries.
	 * @param folder Path to the folder to scan.
	 * @return A vector of plugin file paths.
	 */
	std::vector<std::string> scanPlugins(const std::string &folder);

	/**
	 * @brief Unloads all currently loaded plugins and clears internal state.
	 */
	void unloadAll();

	std::vector<Plugin> plugins_;      ///< Loaded plugins
	std::string         pluginFolder_; ///< Plugin search folder
};

} // namespace Phasor
