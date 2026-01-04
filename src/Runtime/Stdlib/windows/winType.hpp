#pragma once
#include "../../VM/VM.hpp"
#include "../../Value.hpp"
#include <cstdint>

#include <Windows.h>

#include "vhandle.hpp"   // For Virtual Handles

namespace win
{
    // ============================================================
    // strings
    // ============================================================

    // Requires string, throws otherwise
    LPCSTR asLPCSTR(const Value& v);
    LPCWSTR asLPCWSTR(const Value& v);

    // Accepts null or string
    LPCSTR asOptionalLPCSTR(const Value& v);
    LPCWSTR asOptionalLPCWSTR(const Value& v);

    // Mutable buffers when Win32 writes into them
    LPSTR asLPSTR(const Value& v, size_t& capacity);
    LPWSTR asLPWSTR(const Value& v, size_t& capacity);


    // ============================================================
    // value sized integers (32 bit)
    // ============================================================

    DWORD asDWORD(const Value& v);          // flags, sizes, masks
    UINT  asUINT(const Value& v);
    WORD  asWORD(const Value& v);
    BYTE  asBYTE(const Value& v);

    BOOL  asBOOL(const Value& v);           // normalize to 0 or 1

    LONG  asLONG(const Value& v);
    ULONG asULONG(const Value& v);


    // ============================================================
    // pointer sized integers
    // ============================================================

    LPARAM asLPARAM(const Value& v);
    WPARAM asWPARAM(const Value& v);
    LRESULT asLRESULT(const Value& v);

    // Accept only integer semantics, no floats
    intptr_t  asINTPTR(const Value& v);
    uintptr_t asUINTPTR(const Value& v);


    // ============================================================
    // explicit 64 bit
    // ============================================================

    int64_t  asINT64(const Value& v);
    uint64_t asUINT64(const Value& v);


    // ============================================================
    // handles and opaque pointers
    // ============================================================

    template<typename T>
    T asHandle(const Value& v);              // requires GUID

    template<typename T>
    T asOptionalHandle(const Value& v);      // null or GUID

    HANDLE asHANDLE(const Value& v);
    HWND   asHWND(const Value& v);
    HMODULE asHMODULE(const Value& v);


    // ============================================================
    // output conversions
    // ============================================================

    Value fromBOOL(BOOL v);
    Value fromDWORD(DWORD v);
    Value fromUINT(const UINT v);

    Value fromINT64(int64_t v);
    Value fromUINT64(uint64_t v);

    Value wrapKernelHandle(HANDLE h);
    Value wrapUserHandle(HANDLE h);


    // ============================================================
    // raw memory
    // ============================================================

    void*  asBuffer(const Value& v, size_t& size);
    void*  asOptionalBuffer(const Value& v, size_t& size);
}