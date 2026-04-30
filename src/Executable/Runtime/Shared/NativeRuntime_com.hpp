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

    HRESULT QueryInterface(REFIID riid, void** ppv) override;
    ULONG AddRef() override;
    ULONG Release() override;

    HRESULT SetScriptSite(IActiveScriptSite* pSite) override;
    HRESULT GetScriptSite(REFIID riid, void** ppvObject) override;
    HRESULT SetScriptState(SCRIPTSTATE state) override;
    HRESULT GetScriptState(SCRIPTSTATE* pss) override;
    HRESULT Close() override;
    HRESULT AddNamedItem(LPCOLESTR name, DWORD flags) override;
    HRESULT AddTypeLib(REFGUID rguidTypeLib, DWORD dwMajor, DWORD dwMinor, DWORD dwFlags) override;
    HRESULT GetScriptDispatch(LPCOLESTR name, IDispatch** ppdisp) override;
    HRESULT GetCurrentScriptThreadID(SCRIPTTHREADID* pstidThread) override;
    HRESULT GetScriptThreadID(DWORD dwWin32ThreadId, SCRIPTTHREADID* pstidThread) override;
    HRESULT GetScriptThreadState(SCRIPTTHREADID stidThread, SCRIPTTHREADSTATE* pstsState) override;
    HRESULT InterruptScriptThread(SCRIPTTHREADID stidThread, const EXCEPINFO* pexcepinfo, DWORD dwFlags) override;
    HRESULT Clone(IActiveScript** ppscript) override;

    HRESULT InitNew() override;
    HRESULT AddScriptlet(LPCOLESTR defaultName, LPCOLESTR code, LPCOLESTR itemName, LPCOLESTR subItemName,
                         LPCOLESTR eventName, LPCOLESTR delimiter, DWORDLONG sourceContextCookie,
                         ULONG startingLine, DWORD flags, BSTR* pbstrName, EXCEPINFO* pexcepinfo) override;
    HRESULT ParseScriptText(
        LPCOLESTR code,
        LPCOLESTR itemName,
        IUnknown* context,
        LPCOLESTR delimiter,
        DWORDLONG sourceContextCookie,
        ULONG startingLine,
        DWORD flags,
        VARIANT* result,
        EXCEPINFO* ex
    ) override;
};