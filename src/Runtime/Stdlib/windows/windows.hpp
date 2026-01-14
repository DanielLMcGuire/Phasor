#pragma once

#include <Windows.h>

#include "../../VM/VM.hpp" // For function registration
#include "../../Value.hpp" 
#include "../StdLib.hpp"   // For argument checking

#include "vhandle.hpp"     // Safe handle management
#include "winType.hpp"     // Safe win32 type management

#include "winuser.hpp"     // Windows User API functions

#ifndef NOT_BUILD_WINDOWS_DEPRECATE
# define NOT_BUILD_WINDOWS_DEPRECATE __declspec(deprecated)
#endif

class windows
{
public: 
    /// @brief Register Windows API functions
    static Value registerFunctions(const std::vector<Value> &args, VM *vm);
private:
    /// @brief Get the last error code from Windows API
    static Value GetLastError(const std::vector<Value> &args, VM *vm);
    /// @brief Set the last error code for Windows API
    static Value SetLastError(const std::vector<Value> &args, VM *vm);
    /// @brief Get the current thread ID
    static Value GetCurrentThreadId(const std::vector<Value> &args, VM *vm);
    /// @brief Get the current process ID
    static Value GetCurrentProcessId(const std::vector<Value> &args, VM *vm);
    /// @brief Get the tick count since system start
    static Value GetTickCount64(const std::vector<Value> &args, VM *vm);
    /// @brief Get the Windows version
    /// @warning This function has been deprecated by Microsoft.
    static Value GetVersion(const std::vector<Value> &args, VM *vm);
    /// @brief Get the input code page used by the console
    static Value GetConsoleCP(const std::vector<Value> &args, VM *vm);
    /// @brief Get the output code page used by the console
    static Value GetConsoleOutputCP(const std::vector<Value> &args, VM *vm);
    /// @brief Returns a handle to the current process.
    static Value GetCurrentProcess(const std::vector<Value> &args, VM *vm);
    /// @brief Close an open object handle
    static Value CloseHandle(const std::vector<Value> &args, VM *vm);
    /// @brief Load a DLL and return a handle
    /// @note _ours is suffixed to avoid conflict with Windows LoadLibrary macro
    static Value LoadLibrary_ours(const std::vector<Value> &args, VM *vm);
    /// @brief Get a procedure address from a loaded DLL
    static Value GetProcAddress(const std::vector<Value> &args, VM *vm);
    /// @brief Run a procedure from a loaded DLL
    /// @note This is not an official Windows API function, but a helper to call procedures while we wait for VM major update 3
    /// @note which will add call_std to work on mapped C++ functions (like these very ones) instead of call_native
    /// @note which will instead be used for raw pointers.
    static Value RunProcFromAddress(const std::vector<Value> &args, VM *vm);
    /// @brief Create a security descriptor handle
    static Value GetSecurityDescriptorHandle(const std::vector<Value> &args, VM *vm);
    /// @brief Wrap a raw security descriptor pointer
    static Value WrapSecurityDescriptorHandle(const std::vector<Value> &args, VM *vm);
    /// @brief Free security descriptor handle
    static Value FreeSecurityDescriptorHandle(const std::vector<Value> &args, VM *vm);
    /// @brief Create security attributes handle
    static Value GetSecurityAttributesHandle(const std::vector<Value> &args, VM *vm);
    /// @brief Free security attributes handle
    static Value FreeSecurityAttributesHandle(const std::vector<Value> &args, VM *vm);
    /// @brief Create or open a file
    static Value CreateFile_ours(const std::vector<Value> &args, VM *vm);
    /// @brief Free a loaded DLL 
    static Value FreeLibrary(const std::vector<Value> &args, VM *vm);
    /// @brief Generate a beep sound via the PC speaker
    static Value Beep(const std::vector<Value> &args, VM *vm);
};

/// @brief Macro to define a Windows API function that returns a BOOL or BOOLEAN
/// @param fn The function name
#define WIN_BOOL_FN(fn) \
\
Value windows::fn(const std::vector<Value> &args, VM *vm) { \
    StdLib::checkArgCount(args, 0, "win_"#fn); \
    return static_cast<bool>(::fn()); \
}
/// @brief Macro to define a safe String based Windows API function 
/// This will work with the following types: BYTE, WORD, DWORD, CHAR, WCHAR
/// This may work with additional std::string castable types as well.
/// @param fn The function name
#define WIN_STRING_FN(fn) \
Value windows::fn(const std::vector<Value> &args, VM *vm) { \
    StdLib::checkArgCount(args, 0, "win_"#fn); \
    return std::string(::fn()); \
}

/// @brief Macro to define a safe Wide String based Windows API function
/// This will work with the following types: WCHAR, LPWSTR, LPCWSTR
/// This may work with additional WIN32 wide string types as well.
/// @param fn The function name
#define WIN_WSTRING_FN(fn) \
Value windows::fn(const std::vector<Value> &args, VM *vm) { \
    StdLib::checkArgCount(args, 0, "win_"#fn); \
    std::wstring ws = ::fn(); \
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr); \
    std::string str(len, 0); \
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &str[0], len, nullptr, nullptr); \
    return str; \
}

/// @brief Macro to define a Number based Windows API function
/// This will work with the following types: BYTE, WORD, DWORD, UINT, ULONG, ULONGLONG, INT, LONG, LONGLONG
/// @param fn The function name
#define WIN_NUMERAL_FN(fn) \
Value windows::fn(const std::vector<Value> &args, VM *vm) { \
    StdLib::checkArgCount(args, 0, "win_"#fn); \
    return static_cast<int64_t>(::fn()); \
}

/// @brief Macro to define a Windows API function that returns a HANDLE
/// This will work on all handles like HANDLE, HINSTANCE, HMODULE, etc.
/// @param fn The function name
/// @param handleType The type of handle @see vhandle::HandleType
/// @param vmOwn Whether the VM owns the HANDLE and can free it
#define WIN_HANDLE_FUNCTION(fn, handleType, vmOwn) \
Value windows::fn(const std::vector<Value> &args, VM *vm) { \
    StdLib::checkArgCount(args, 0, "win_" #fn); \
    vhandle::VHANDLE handle = vhandle::createVHandle(::fn(), vhandle::HandleType::handleType, vmOwn); \
    return vhandle::vhandleGuid(handle); \
}