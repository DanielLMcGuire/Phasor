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

namespace Phasor
{
/**
 * @brief Converts a C-style FFI value to a C++ VM value.
 * @param c_value The C-style value from the plugin.
 * @return The equivalent C++ value for the VM.
 * @note This function is safe for strings, as it copies the C string into a
 *       new C++ std::string, taking ownership of the memory.
 */
static Phasor::Value from_c_value(const PhasorValue& c_value)
{
    switch (c_value.type)
    {
    case PHASOR_TYPE_NULL:
        return Phasor::Value();
    case PHASOR_TYPE_BOOL:
        return Phasor::Value(c_value.as.b);
    case PHASOR_TYPE_INT:
        return Phasor::Value(c_value.as.i);
    case PHASOR_TYPE_FLOAT:
        return Phasor::Value(c_value.as.f);
    case PHASOR_TYPE_STRING:
        if (c_value.as.s) {
            return Phasor::Value(c_value.as.s);
        }
        return Phasor::Value("");
    default:
        return Phasor::Value();
    }
}

/**
 * @brief Converts a C++ VM value to a C-style FFI value.
 * @param cpp_value The C++ value from the VM.
 * @param string_arena A vector of unique_ptrs to manage the lifetime of C strings.
 *                     Any strings converted will be allocated and their memory will be
 *                     managed by this arena.
 * @return The equivalent C-style value for the plugin.
 */
static PhasorValue to_c_value(const Phasor::Value& cpp_value,
                              std::vector<std::unique_ptr<char[]>>& string_arena)
{
    switch (cpp_value.getType())
    {
    case ValueType::Null:
        return phasor_make_null();
    case ValueType::Bool:
        return phasor_make_bool(cpp_value.asBool());
    case ValueType::Int:
        return phasor_make_int(cpp_value.asInt());
    case ValueType::Float:
        return phasor_make_float(cpp_value.asFloat());
    case ValueType::String:
    {
        const auto& str = cpp_value.asString();
        auto c_str = std::make_unique<char[]>(str.length() + 1);
        std::copy(str.begin(), str.end(), c_str.get());
        c_str[str.length()] = '\0';
        PhasorValue val = phasor_make_string(c_str.get());
        string_arena.push_back(std::move(c_str));
        return val;
    }
    default:
        return phasor_make_null();
    }
}

// =============================================================================
// FFI Host Implementation
// =============================================================================

/**
 * @brief The "trampoline" that wraps a C function from a plugin.
 *
 * This function is the bridge between the C++ VM and the C plugin. It is called
 * by the VM and is responsible for:
 * 1. Converting C++ arguments to C arguments, with production-ready memory management for strings.
 * 2. Calling the plugin's C function.
 * 3. Converting the C return value back to a C++ value.
 */
static Phasor::Value c_native_func_wrapper(PhasorNativeFunction c_func, Phasor::VM* vm, const std::vector<Phasor::Value>& args)
{
    std::vector<std::unique_ptr<char[]>> string_arena;

    std::vector<PhasorValue> c_args;
    c_args.reserve(args.size());
    for (const auto& arg : args)
    {
        c_args.push_back(to_c_value(arg, string_arena));
    }

    PhasorValue c_result = c_func(reinterpret_cast<PhasorVM*>(vm), (int)c_args.size(), c_args.data());

    return from_c_value(c_result);
}

/**
 * @brief The concrete implementation of the `PhasorRegisterFunction` API call.
 *
 * This function is passed to the plugin. When the plugin calls it, this function
 * creates a C++ lambda that wraps the plugin's C function pointer and registers
 * that lambda with the Phasor VM.
 */
static void register_native_c_func(PhasorVM* vm, const char* name, PhasorNativeFunction func)
{
    Phasor::VM* cpp_vm = reinterpret_cast<Phasor::VM*>(vm);

    auto wrapper = [func](const std::vector<Phasor::Value>& args, Phasor::VM* vm_param) -> Phasor::Value
    {
        return c_native_func_wrapper(func, vm_param, args);
    };

    cpp_vm->registerNativeFunction(name, wrapper);
}

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

/**
 * @brief Scans configured plugin directories for shared libraries.
 */
std::vector<std::string> FFI::scanPlugins(const std::string &folder)
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
#else
	char path[1024];
	ssize_t count = readlink("/proc/self/exe", path, sizeof(path));
	if (count != -1)
		exeDir = std::filesystem::path(std::string(path, count)).parent_path();
	else
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

FFI::FFI(const std::string &pluginFolder, VM *vm) : pluginFolder_(pluginFolder)
{
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

} // namespace Phasor
