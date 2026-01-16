#pragma once
#define _WINSOCKAPI_
#include <windows.h>

#include <cstring>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <stdint.h>
#include <stdexcept>
#include <cstdio>

struct GUIDHash
{
	size_t operator()(const GUID &g) const noexcept
	{
		size_t h = 0;

		auto mix = [&](size_t v) { h ^= v + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2); };

		mix(std::hash<uint32_t>{}(g.Data1));
		mix(std::hash<uint16_t>{}(g.Data2));
		mix(std::hash<uint16_t>{}(g.Data3));

		for (uint8_t b : g.Data4)
		{
			mix(std::hash<uint8_t>{}(b));
		}

		return h;
	}
};

struct GUIDEqual
{
	bool operator()(const GUID &a, const GUID &b) const noexcept
	{
		return a.Data1 == b.Data1 && a.Data2 == b.Data2 && a.Data3 == b.Data3 && std::memcmp(a.Data4, b.Data4, 8) == 0;
	}
};

namespace Phasor
{

/// @brief Phasor Sandboxed Windows Handle Management
namespace vhandle
{
/// @brief Virtual Handle (GUID)
typedef GUID VHANDLE;

/// @brief Types of handles
enum class HandleType
{
	Kernel,    ///< HANDLE, HINSTANCE, etc.
	Module,    ///< HMODULE
	Window,    ///< HWND
	Socket,    ///< SOCKET
	Com,       ///< COM object
	Procedure, ///< FARPROC
	Generic,   ///< Generic pointer handle. Used for our own handles.
	/// @todo Move pointer handles to the VM, use them as the base for posix api and win32 apis
};

/// @brief Entry in the vhandle map
struct HandleEntry
{
	void      *handle;    ///< The real HANDLE
	HandleType type;      ///< The type of handle
	bool       isVMOwned; ///< Whether the VM owns the handle and can free it
};

/// @brief Convert a vhandle (GUID) to it's string representation
/// @param vhandle The vhandle to convert
/// @return The string representation of the vhandle
std::string vhandleGuid(const VHANDLE &vhandle);

/// @brief Convert a string representation of a vhandle (GUID) to a vhandle
/// @param guidStr The string representation of the vhandle
/// @return The handle (GUID)
vhandle::VHANDLE guidToVHandle(const std::string &guidStr);

/// @brief Create a new vhandle (GUID) from a real HANDLE and return the new vhandle
/// @param handle The real HANDLE to associate
/// @param type The type of handle @see HandleType
/// @param isVMOwned VM Ownership @see HandleEntry::isVMOwned
/// @return The created vhandle (GUID)
vhandle::VHANDLE createVHandle(const void *handle, HandleType type, const bool isVMOwned = true);

/// @brief Get a HANDLE from a vhandle (GUID)
/// @param handle The vhandle to get the HANDLE for
/// @return The real HANDLE associated with the vhandle
template <typename T> T getHandle(const VHANDLE &handle);

/// @brief Free a real HANDLE associated with a vhandle (GUID)
/// @param vhandle The vhandle to free the associated HANDLE for
void freeVHandle(const VHANDLE &vhandle);

extern std::unordered_map<VHANDLE, HandleEntry, GUIDHash, GUIDEqual> vHandleMap;
} // namespace vhandle

} // namespace Phasor