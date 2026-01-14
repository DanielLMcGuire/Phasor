// Purpose: Windows API Bindings for Phasor Programming Language
// Requirements for contributing: Microsoft Programming Style 

#include "windows.hpp"

Value windows::registerFunctions(const std::vector<Value> &args, VM *vm)
{
    StdLib::checkArgCount(args, 0, "include_win32api");
    vm->registerNativeFunction("win_GetLastError", GetLastError);
    vm->registerNativeFunction("win_SetLastError", SetLastError);
    vm->registerNativeFunction("win_GetCurrentThreadId", GetCurrentThreadId);
    vm->registerNativeFunction("win_GetCurrentProcessId", GetCurrentProcessId);
    vm->registerNativeFunction("win_GetTickCount", GetTickCount64);
    vm->registerNativeFunction("win_GetVersion", GetVersion);
    vm->registerNativeFunction("win_GetConsoleCP", GetConsoleCP);
    vm->registerNativeFunction("win_GetConsoleOutputCP", GetConsoleOutputCP);
    vm->registerNativeFunction("win_GetCurrentProcess", GetCurrentProcess);
    vm->registerNativeFunction("win_CloseHandle", CloseHandle);
    vm->registerNativeFunction("win_LoadLibrary", LoadLibrary_ours);
    vm->registerNativeFunction("win_FreeLibrary", FreeLibrary);
    vm->registerNativeFunction("win_GetProcAddress", GetProcAddress);
    vm->registerNativeFunction("win_RunProcFromAddress", RunProcFromAddress);
    vm->registerNativeFunction("win_GetSecurityDescriptorHandle", GetSecurityDescriptorHandle);
    vm->registerNativeFunction("win_WrapSecurityDescriptorHandle", WrapSecurityDescriptorHandle);
    vm->registerNativeFunction("win_FreeSecurityDescriptorHandle", FreeSecurityDescriptorHandle);
    vm->registerNativeFunction("win_GetSecurityAttributesHandle", GetSecurityAttributesHandle);
    vm->registerNativeFunction("win_FreeSecurityAttributesHandle", FreeSecurityAttributesHandle);
    vm->registerNativeFunction("win_CreateFile", CreateFile_ours);
    vm->registerNativeFunction("win_Beep", Beep);
    vm->registerNativeFunction("include_winuser", winuser::registerFunctions);

    return true;
}
#pragma region Common
// _Post_equals_last_error_ int<DWORD> GetLastError(); 
WIN_NUMERAL_FN(GetLastError)

// VOID SetLastError([in] DWORD<int> dwErrCode);
Value windows::SetLastError(const std::vector<Value> &args, VM *vm)
{
    StdLib::checkArgCount(args, 1, "win_SetLastError");
    DWORD dwErrCode = win::asDWORD(args[0]);
    ::SetLastError(dwErrCode);
    return true;
}

// int<DWORD> GetCurrentThreadId();
WIN_NUMERAL_FN(GetCurrentThreadId)

// int<DWORD> GetCurrentProcessId();
WIN_NUMERAL_FN(GetCurrentProcessId)

// int<ULONGLONG> GetTickCount64();
WIN_NUMERAL_FN(GetTickCount64)

// NOT_BUILD_WINDOWS_DEPRECATE int<DWORD> GetVersion();
#pragma warning(push)
#pragma warning(disable: 4996)
WIN_NUMERAL_FN(GetVersion);
#pragma warning(pop)

// int<UINT> WINAPI GetConsoleCP(void);
WIN_NUMERAL_FN(GetConsoleCP)

// int<UINT> WINAPI GetConsoleOutputCP(void);
WIN_NUMERAL_FN(GetConsoleOutputCP)

// VHANDLE<KERNEL> GetCurrentProcess();
WIN_HANDLE_FUNCTION(GetCurrentProcess, Kernel, false)

// bool<BOOL> Beep(DWORD<int> dwFreq, DWORD<int> dwDuration);
Value windows::Beep(const std::vector<Value> &args, VM *vm)
{
    StdLib::checkArgCount(args, 2, "win_Beep");
    DWORD dwFreq = win::asDWORD(args[0]);
    if (dwFreq < 37 || dwFreq > 32767) {
        throw std::runtime_error("win_Beep: Frequency must be between 37 and 32767 Hz");
    }
    DWORD dwDuration = win::asDWORD(args[1]);
    BOOL result = ::Beep(dwFreq, dwDuration);
    return win::fromBOOL(result);
}
#pragma endregion // common
#pragma region Security
Value windows::GetSecurityDescriptorHandle(const std::vector<Value> &args, VM *vm)
{
    StdLib::checkArgCount(args, 0, "win_GetSecurityDescriptorHandle");

    SECURITY_DESCRIPTOR* sd = new SECURITY_DESCRIPTOR();

    if (!InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION)) {
        delete sd;
        throw std::runtime_error("win_GetSecurityDescriptorHandle: Failed to initialize security descriptor");
    }

    return vhandle::vhandleGuid(
        vhandle::createVHandle(sd, vhandle::HandleType::Generic, false)
    );
}

Value windows::WrapSecurityDescriptorHandle(const std::vector<Value> &args, VM *vm)
{
    StdLib::checkArgCount(args, 1, "win_WrapSecurityDescriptorHandle");

    void* rawPtr = reinterpret_cast<void*>(win::asUINTPTR(args[0]));

    if (!rawPtr) {
        throw std::runtime_error("win_WrapSecurityDescriptorHandle: Invalid raw pointer");
    }

    return vhandle::vhandleGuid(
        vhandle::createVHandle(
            reinterpret_cast<SECURITY_DESCRIPTOR*>(rawPtr),
            vhandle::HandleType::Generic,
            false
        )
    );
}

Value windows::FreeSecurityDescriptorHandle(const std::vector<Value> &args, VM *vm)
{
    StdLib::checkArgCount(args, 1, "win_FreeSecurityDescriptorHandle");
    vhandle::VHANDLE vhandle = vhandle::guidToVHandle(args[0].asString());
    SECURITY_DESCRIPTOR* sd = vhandle::getHandle<SECURITY_DESCRIPTOR*>(vhandle);
    delete sd;
    vhandle::freeVHandle(vhandle);
    return true;
}

// typedef struct _SECURITY_ATTRIBUTES { DWORD  nLength; LPVOID lpSecurityDescriptor; BOOL bInheritHandle; } *SECURITY_ATTRIBUTES;
// SHAPED:
// VHANDLE<GENERIC> GetSecurityAttributesHandle([in] DWORD<int> nLength, [in] VHANDLE<GENERIC> lpSecurityDescriptor, [in] BOOL<bool> bInheritHandle);
Value windows::GetSecurityAttributesHandle(const std::vector<Value> &args, VM *vm)
{
    StdLib::checkArgCount(args, 3, "win_GetSecurityAttributesHandle");
    DWORD nLength = win::asDWORD(args[0]);
    void* lpSecurityDescriptor = nullptr;
    if (!args[1].isNull()) {
        vhandle::VHANDLE sdHandle = vhandle::guidToVHandle(args[1].asString());
        lpSecurityDescriptor = vhandle::getHandle<void*>(sdHandle);
    } else {
        return vhandle::vhandleGuid(vhandle::createVHandle(nullptr, vhandle::HandleType::Generic, false));
    }
    BOOL bInheritHandle = win::asBOOL(args[2]);

    SECURITY_ATTRIBUTES* secAttr = new SECURITY_ATTRIBUTES();
    secAttr->nLength = nLength;
    secAttr->lpSecurityDescriptor = lpSecurityDescriptor;
    secAttr->bInheritHandle = bInheritHandle;

    return vhandle::vhandleGuid(vhandle::createVHandle(secAttr, vhandle::HandleType::Generic, false));
}

Value windows::FreeSecurityAttributesHandle(const std::vector<Value> &args, VM *vm) 
{
    StdLib::checkArgCount(args, 1, "win_FreeSecurityAttributesHandle");
    vhandle::VHANDLE     vhandle = vhandle::guidToVHandle(args[0].asString());
    SECURITY_ATTRIBUTES* secAttr = vhandle::getHandle<SECURITY_ATTRIBUTES*>(vhandle);
    delete secAttr;
    vhandle::freeVHandle(vhandle);
    return true;

}
#pragma endregion // Security
#pragma region File
// VHANDLE<KERNEL> CreateFileA([in] LPCSTR<string> lpFileName, [in] DWORD<int> dwDesiredAccess, [in] DWORD<int> dwShareMode,
//                     [in, optional] LPSECURITY_ATTRIBUTES lpSecurityAttributes, [in] DWORD<int> dwCreationDisposition,
//                     [in] DWORD<int> dwFlagsAndAttributes, [in, optional] VHANDLE<KERNEL> hTemplateFile);
Value windows::CreateFile_ours(const std::vector<Value> &args, VM *vm)
{
    StdLib::checkArgCount(args, 6, "win_CreateFile", true);
    LPCSTR lpFileName = win::asLPCSTR(args[0]);
    DWORD dwDesiredAccess = win::asDWORD(args[1]);
    DWORD dwShareMode = win::asDWORD(args[2]);
    vhandle::VHANDLE lpSecurityAttributes = vhandle::guidToVHandle(args[3].asString());
    LPSECURITY_ATTRIBUTES lpSecurityAttributesPtr = vhandle::getHandle<LPSECURITY_ATTRIBUTES>(lpSecurityAttributes);
    DWORD dwCreationDisposition = win::asDWORD(args[4]);
    DWORD dwFlagsAndAttributes = win::asDWORD(args[5]);
    HANDLE hTemplateFile = win::asOptionalHandle<HANDLE>(args[6]);

    HANDLE hFile = ::CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributesPtr,
                                 dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    if (hFile == INVALID_HANDLE_VALUE)
        throw std::runtime_error("win_CreateFileA: Failed to create or open file");
    return win::wrapKernelHandle(hFile);
}
#pragma endregion // File
#pragma region Library / Handles
// bool<BOOL> CloseHandle([in] HANDLE hObject);
Value windows::CloseHandle(const std::vector<Value> &args, VM *vm)
{
    StdLib::checkArgCount(args, 1, "win_CloseHandle");
    vhandle::freeVHandle(vhandle::guidToVHandle(args[0].asString()));
    return true;
}

// VHANDLE<MODULE> LoadLibraryA([in] LPCSTR<string> lpLibFileName);
Value windows::LoadLibrary_ours(const std::vector<Value> &args, VM *vm)
{
    StdLib::checkArgCount(args, 1, "win_LoadLibrary");
    LPCSTR libName = win::asLPCSTR(args[0]);
    HMODULE lib = ::LoadLibraryA(libName);
    if (lib == NULL) {
        throw std::runtime_error("win_LoadLibrary: Failed to load library");
    }
    return vhandle::vhandleGuid(vhandle::createVHandle(lib, vhandle::HandleType::Module, false));
}

// VHANDLE<PROCEDURE> GetProcAddress(VHANDLE<MODULE> hModule, LPCSTR<string> lpProcName);
Value windows::GetProcAddress(const std::vector<Value> &args, VM *vm) {
    StdLib::checkArgCount(args, 2, "win_GetProcAddress");
    HMODULE hModule = win::asHMODULE(args[0]);
    LPCSTR procName = win::asLPCSTR(args[1]);

    FARPROC procAddr = ::GetProcAddress(hModule, procName);
    if (!procAddr) {
        throw std::runtime_error("win_GetProc: Failed to get procedure address for " + args[1].asString());
    }

    return vhandle::vhandleGuid(vhandle::createVHandle(reinterpret_cast<void*>(procAddr), vhandle::HandleType::Procedure, true));
}

// dynamic_type RunProcFromAddress(VHANDLE<PROCEDURE> procAddr, args...);
Value windows::RunProcFromAddress(const std::vector<Value> &args, VM *vm) {
    vhandle::VHANDLE handle   = vhandle::guidToVHandle(args[0].asString());
    FARPROC          procAddr = vhandle::getHandle<FARPROC>(handle);

    std::vector<int> intArgs;
    for (size_t i = 1; i < args.size(); ++i) {
        intArgs.push_back(static_cast<int>(args[i].asInt()));
    }

    auto callProc = [procAddr, &intArgs]() -> int {
        using GenericFunc = int(*)(const int*, size_t);
        GenericFunc wrapper = reinterpret_cast<GenericFunc>(reinterpret_cast<void*>(procAddr));
        return wrapper(intArgs.data(), intArgs.size());
    };

    try {
        return callProc();
    } catch (...) {
        throw std::runtime_error("win_RunProc: Failed to call procedure");
    }
}

// bool<BOOL> FreeLibrary(VHANDLE<MODULE> hModule);
Value windows::FreeLibrary(const std::vector<Value> &args, VM *vm)
{
    StdLib::checkArgCount(args, 1, "win_FreeLibrary");
    vhandle::VHANDLE vhandle = vhandle::guidToVHandle(args[0].asString());
    HMODULE hModule = vhandle::getHandle<HMODULE>(vhandle);
    BOOL result = ::FreeLibrary(hModule);
    vhandle::freeVHandle(vhandle);
    return win::fromBOOL(result);
}
#pragma endregion // Library / Handles
