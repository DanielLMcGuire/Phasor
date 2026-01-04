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

/// @brief Hash operator for GUID
struct GUIDHash {
    size_t operator()(const GUID &g) const {
        const uint64_t *p = reinterpret_cast<const uint64_t*>(&g);
        return std::hash<uint64_t>()(p[0]) ^ std::hash<uint64_t>()(p[1]);
    }
};
/// @brief Equality operator for GUID
struct GUIDEqual {
    bool operator()(const GUID &a, const GUID &b) const {
        return memcmp(&a, &b, sizeof(GUID)) == 0;
    }
};

/// @namespace vhandle
/// @brief Phasor Sandboxed Windows Handle Management
/// This namespace provides functionality to create and manage virtual handles (vhandles)
/// that map to real Windows HANDLEs. This allows safe management of Windows resources
namespace vhandle {
    /// @brief Virtual Handle (GUID)
    typedef GUID VHANDLE;

    /// @brief Types of handles
    enum class HandleType {
        Kernel, ///< HANDLE, HINSTANCE, etc.
        Module, ///< HMODULE
        Window, ///< HWND
        Socket, ///< SOCKET
        Com, ///< COM object
        Procedure, ///< FARPROC
        Generic, ///< Generic pointer handle. Used for our own handles. 
        /// @todo Move pointer handles to the VM, use them as the base for posix api and win32 apis  
    };

    /// @brief Entry in the vhandle map
    struct HandleEntry {
        void* handle; ///< The real HANDLE
        HandleType type; ///< The type of handle
        bool isVMOwned; ///< Whether the VM owns the handle and can free it
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
    vhandle::VHANDLE createVHandle(const void* handle, HandleType type, const bool isVMOwned = true);

    /// @brief Get a HANDLE from a vhandle (GUID)
    /// @param handle The vhandle to get the HANDLE for
    /// @return The real HANDLE associated with the vhandle
    template<typename T>
    T getHandle(const VHANDLE &handle);

    /// @brief Free a real HANDLE associated with a vhandle (GUID)
    /// @param vhandle The vhandle to free the associated HANDLE for
    void freeVHandle(const VHANDLE &vhandle);

    extern std::unordered_map<VHANDLE, HandleEntry, GUIDHash, GUIDEqual> vHandleMap;
}