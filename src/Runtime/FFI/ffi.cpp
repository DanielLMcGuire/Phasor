#include "ffi.hpp"
#include <vector>
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include <sstream>
#include <filesystem>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

#define INSTANCED_FFI(fn) [this](const std::vector<Value> &args, VM *vm) { return this->fn(args, vm); }

#define VM_PRINT(str) vm->setRegister(VM::r0, str); vm->operation(OpCode::PRINT_R, VM::r0);

namespace Phasor
{

/**
 * @brief Loads a plugin, finds its entry point, and initializes it.
 */
bool FFI::loadPlugin(const std::filesystem::path &library, VM *vm)
{
    using PluginEntryFunc = void (*)(const PhasorAPI*, PhasorVM*);

#if defined(_WIN32)
	HMODULE lib = LoadLibraryA(library.string().c_str());
	if (!lib) {
		std::stringstream ss;
		ss << "FFI Error: Failed to load library " << library.string() << ". Code: " << GetLastError();
        throw std::runtime_error(ss.str());
		return false;
    }
	auto entry_point = (PluginEntryFunc)GetProcAddress(lib, "phasor_plugin_entry");
#else
	void *lib = dlopen(library.string().c_str(), RTLD_NOW);
	if (!lib) {
		std::stringstream ss;
		ss << "FFI Error: Failed to load library " << library.string() << ". Error: " << dlerror();
		throw std::runtime_error(ss.str());
		return false;
    }
	auto entry_point = (PluginEntryFunc)dlsym(lib, "phasor_plugin_entry");
#endif

	if (!entry_point)
	{
		throw std::runtime_error(std::string("FFI Error: Could not find entry point 'phasor_plugin_entry' in " + library.string()));
#if defined(_WIN32)
		FreeLibrary(lib);
#else
		dlclose(lib);
#endif
		return false;
	}

    PhasorAPI api;
    api.register_function = &register_native_c_func;

    entry_point(&api, reinterpret_cast<PhasorVM*>(vm));

	plugins_.push_back(Plugin{lib, library.string(), nullptr});
	return true;
}

bool FFI::addPlugin(const std::filesystem::path &pluginPath)
{
    return loadPlugin(pluginPath, vm_);
}

/**
 * @brief Scans configured plugin directories for shared libraries.
 */
std::vector<std::string> FFI::scanPlugins(const std::filesystem::path &folder)
{
	if (folder.empty()) return {};

	std::vector<std::string> plugins;
	std::filesystem::path exeDir;
	std::vector<std::filesystem::path> foldersToScan;

#if defined(_WIN32)
	char path[MAX_PATH];
	GetModuleFileNameA(nullptr, path, MAX_PATH);
	exeDir = std::filesystem::path(path).parent_path();
#elif defined(__APPLE__)
	char path[1024];
	uint32_t size = sizeof(path);
	if (_NSGetExecutablePath(path, &size) == 0)
		exeDir = std::filesystem::path(path).parent_path();
	else
		exeDir = std::filesystem::current_path();
#elif defined(__linux__)
	char path[1024];
	ssize_t count = readlink("/proc/self/exe", path, sizeof(path));
	if (count != -1)
		exeDir = std::filesystem::path(std::string(path, count)).parent_path();
	else
		exeDir = std::filesystem::current_path();
#else
	exeDir = std::filesystem::current_path();
#endif


	foldersToScan.push_back(exeDir / folder);
    if (!std::filesystem::equivalent(exeDir, std::filesystem::current_path())) {
	    foldersToScan.push_back(std::filesystem::current_path() / folder);
    }

	for (auto &folderPath : foldersToScan)
	{
		if (!std::filesystem::exists(folderPath) || !std::filesystem::is_directory(folderPath))
			continue;

		for (auto &p : std::filesystem::directory_iterator(folderPath))
		{
            if (!p.is_regular_file()) continue;
			auto ext = p.path().extension().string();
#if defined(_WIN32)
			if (ext == ".dll" || ext == ".phsp")
#elif defined(__APPLE__)
			if (ext == ".dylib" || ext == ".phsp")
#else
			if (ext == ".so" || ext == ".phsp")
#endif
				plugins.push_back(p.path().string());
		}
	}
	return plugins;
}

/**
 * @brief Unloads all currently loaded plugins.
 */
void FFI::unloadAll()
{
	for (auto &plugin : plugins_)
	{
#if defined(_WIN32)
		FreeLibrary(plugin.handle);
#else
		dlclose(plugin.handle);
#endif
	}
	plugins_.clear();
}

FFI::FFI(const std::filesystem::path &pluginFolder, VM *vm) : pluginFolder_(pluginFolder), vm_(vm)
{
	vm->registerNativeFunction("load_plugin", INSTANCED_FFI(FFI::native_add_plugin));
    auto plugins = scanPlugins(pluginFolder_);
    for (const auto &pluginPath : plugins)
    {
        try
        {
            loadPlugin(pluginPath, vm);
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
}

FFI::~FFI()
{
    unloadAll();
}

Value FFI::native_add_plugin(const std::vector<Value> &args, VM *vm)
{
	if (args.size() != 1)
    {
        throw std::runtime_error("load_plugin expects exactly 1 argument: the plugin path.");
    }
	std::filesystem::path pluginPath = args[0].asString();
    if (!std::filesystem::exists(pluginPath))
    {
        throw std::runtime_error("Plugin file does not exist: " + pluginPath.string());
	}
    return addPlugin(pluginPath);
}
} // namespace Phasor
