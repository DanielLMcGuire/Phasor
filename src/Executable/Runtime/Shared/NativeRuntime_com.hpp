#pragma once
#include <Windows.h>
#include <activscp.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "../../../Runtime/VM/VM.hpp"

// {c5318f33-2d87-4e95-95b7-2928cd57a5d7}
static const GUID CLSID_PhasorEngine = { 0xc5318f33, 0x2d87, 0x4e95, { 0x95, 0xb7, 0x29, 0x28, 0xcd, 0x57, 0xa5, 0xd7 } };

class PhasorScriptEngine final :
    public IActiveScript,
    public IActiveScriptParse
{
    long refCount = 1;

    Phasor::VM vm;

    SCRIPTSTATE state = SCRIPTSTATE_UNINITIALIZED;
    IActiveScriptSite* site = nullptr;

public:
    virtual ~PhasorScriptEngine() = default;

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override;
    ULONG __stdcall AddRef() override;
    ULONG __stdcall Release() override;

    HRESULT __stdcall SetScriptSite(IActiveScriptSite* pSite) override;
    HRESULT __stdcall GetScriptSite(REFIID riid, void** ppvObject) override;
    HRESULT __stdcall SetScriptState(SCRIPTSTATE state) override;
    HRESULT __stdcall GetScriptState(SCRIPTSTATE* pss) override;
    HRESULT __stdcall Close() override;
    HRESULT __stdcall AddNamedItem(LPCOLESTR name, DWORD flags) override;
    HRESULT __stdcall AddTypeLib(REFGUID rguidTypeLib, DWORD dwMajor, DWORD dwMinor, DWORD dwFlags) override;
    HRESULT __stdcall GetScriptDispatch(LPCOLESTR name, IDispatch** ppdisp) override;
    HRESULT __stdcall GetCurrentScriptThreadID(SCRIPTTHREADID* pstidThread) override;
    HRESULT __stdcall GetScriptThreadID(DWORD dwWin32ThreadId, SCRIPTTHREADID* pstidThread) override;
    HRESULT __stdcall GetScriptThreadState(SCRIPTTHREADID stidThread, SCRIPTTHREADSTATE* pstsState) override;
    HRESULT __stdcall InterruptScriptThread(SCRIPTTHREADID stidThread, const EXCEPINFO* pexcepinfo, DWORD dwFlags) override;
    HRESULT __stdcall Clone(IActiveScript** ppscript) override;

    HRESULT __stdcall InitNew() override;
    HRESULT __stdcall AddScriptlet(LPCOLESTR defaultName, LPCOLESTR code, LPCOLESTR itemName, LPCOLESTR subItemName,
                         LPCOLESTR eventName, LPCOLESTR delimiter, 
#if defined(_WIN64)
                         DWORDLONG sourceContextCookie,
#else
                         DWORD sourceContextCookie,
#endif
                         ULONG startingLine, DWORD flags, BSTR* pbstrName, EXCEPINFO* pexcepinfo) override;
    HRESULT __stdcall ParseScriptText(
        LPCOLESTR code,
        LPCOLESTR itemName,
        IUnknown* context,
        LPCOLESTR delimiter,
#if defined(_WIN64)
        DWORDLONG sourceContextCookie,
#else
        DWORD sourceContextCookie,
#endif
        ULONG startingLine,
        DWORD flags,
        VARIANT* result,
        EXCEPINFO* ex
    ) override;
};