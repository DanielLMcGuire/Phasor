#include "NativeRuntime_com.hpp"

#include "../../../Frontend/Phasor/Frontend.hpp"
#include "../../../Runtime/Stdlib/StdLib.hpp"
#include "../../../Runtime/VM/VM.hpp"

#include <oleauto.h>
#include <string>
#include <unordered_map>
#include <functional>

namespace
{
	std::string wideToUtf8(LPCOLESTR text)
	{
		if (!text)
			return {};

		int required = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
		if (required <= 1)
			return {};

		std::string out(static_cast<size_t>(required - 1), '\0');
		WideCharToMultiByte(CP_UTF8, 0, text, -1, out.data(), required, nullptr, nullptr);
		return out;
	}
}

HRESULT __stdcall PhasorScriptEngine::QueryInterface(REFIID riid, void** ppv)
{
	if (!ppv)
		return E_POINTER;

	*ppv = nullptr;

	if (riid == IID_IUnknown || riid == IID_IActiveScript)
	{
		*ppv = static_cast<IActiveScript*>(this);
	}
	else if (riid == IID_IActiveScriptParse)
	{
		*ppv = static_cast<IActiveScriptParse*>(this);
	}
	else
	{
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}

ULONG __stdcall PhasorScriptEngine::AddRef()
{
	return ++refCount;
}

ULONG __stdcall PhasorScriptEngine::Release()
{
	long r = --refCount;
	if (r == 0)
		delete this;
	return r;
}

HRESULT __stdcall PhasorScriptEngine::SetScriptSite(IActiveScriptSite* pSite)
{
	if (pSite)
		pSite->AddRef();

	if (site)
		site->Release();

	site = pSite;
	return S_OK;
}

HRESULT __stdcall PhasorScriptEngine::GetScriptSite(REFIID riid, void** ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	*ppvObject = nullptr;

	if (!site)
		return E_FAIL;

	return site->QueryInterface(riid, ppvObject);
}

HRESULT __stdcall PhasorScriptEngine::SetScriptState(SCRIPTSTATE st)
{
	state = st;
	return S_OK;
}

HRESULT __stdcall PhasorScriptEngine::GetScriptState(SCRIPTSTATE* pss)
{
	if (!pss)
		return E_POINTER;

	*pss = state;
	return S_OK;
}

HRESULT __stdcall PhasorScriptEngine::Close()
{
	if (site)
	{
		site->Release();
		site = nullptr;
	}

	vm.reset(true, true, true);
	state = SCRIPTSTATE_CLOSED;
	return S_OK;
}

HRESULT __stdcall PhasorScriptEngine::AddNamedItem(LPCOLESTR, DWORD)
{
    return E_NOTIMPL;
}

HRESULT __stdcall PhasorScriptEngine::AddTypeLib(REFGUID, DWORD, DWORD, DWORD)
{
	return E_NOTIMPL;
}

HRESULT __stdcall PhasorScriptEngine::GetScriptDispatch(LPCOLESTR, IDispatch** ppdisp)
{
	if (!ppdisp)
		return E_POINTER;

	*ppdisp = nullptr;
	return E_NOTIMPL;
}

HRESULT __stdcall PhasorScriptEngine::GetCurrentScriptThreadID(SCRIPTTHREADID* pstidThread)
{
	if (!pstidThread)
		return E_POINTER;

	*pstidThread = SCRIPTTHREADID_BASE;
	return S_OK;
}

HRESULT __stdcall PhasorScriptEngine::GetScriptThreadID(DWORD, SCRIPTTHREADID* pstidThread)
{
	if (!pstidThread)
		return E_POINTER;

	*pstidThread = SCRIPTTHREADID_BASE;
	return S_OK;
}

HRESULT __stdcall PhasorScriptEngine::GetScriptThreadState(SCRIPTTHREADID, SCRIPTTHREADSTATE* pstsState)
{
	if (!pstsState)
		return E_POINTER;

	*pstsState = SCRIPTTHREADSTATE_NOTINSCRIPT;
	return S_OK;
}

HRESULT __stdcall PhasorScriptEngine::InterruptScriptThread(SCRIPTTHREADID, const EXCEPINFO*, DWORD)
{
	return E_NOTIMPL;
}

HRESULT __stdcall PhasorScriptEngine::Clone(IActiveScript** ppscript)
{
	if (ppscript)
		*ppscript = nullptr;
	return E_NOTIMPL;
}

HRESULT __stdcall PhasorScriptEngine::InitNew()
{
	vm.reset(true, true, true);
	Phasor::StdLib::registerFunctions(vm);
	state = SCRIPTSTATE_INITIALIZED;
	return S_OK;
}

HRESULT __stdcall PhasorScriptEngine::AddScriptlet(
	LPCOLESTR,
	LPCOLESTR,
	LPCOLESTR,
	LPCOLESTR,
	LPCOLESTR,
	LPCOLESTR,
#if defined(_WIN64)
	DWORDLONG,
#else
	DWORD,
#endif
	ULONG,
	DWORD,
	BSTR* pbstrName,
	EXCEPINFO*)
{
	if (pbstrName)
		*pbstrName = nullptr;

	return E_NOTIMPL;
}

HRESULT __stdcall PhasorScriptEngine::ParseScriptText(
	LPCOLESTR code,
	LPCOLESTR,
	IUnknown*,
	LPCOLESTR,
#if defined(_WIN64)
	DWORDLONG,
#else
	DWORD,
#endif
	ULONG,
	DWORD,
	VARIANT* result,
	EXCEPINFO* ex)
{
	if (!code)
		return E_POINTER;

	if (result)
		VariantInit(result);

	if (ex)
		ZeroMemory(ex, sizeof(*ex));

	try
	{
		std::string src = wideToUtf8(code);
		Phasor::Frontend::runScript(src.c_str(), &vm, "", false);
		return S_OK;
	}
	catch (...)
	{
		return E_FAIL;
	}
}

class ClassFactory final : public IClassFactory
{
	long refCount = 1;

public:
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppv) override
	{
		if (!ppv)
			return E_POINTER;

		*ppv = nullptr;

		if (riid == IID_IUnknown || riid == IID_IClassFactory)
		{
			*ppv = static_cast<IClassFactory*>(this);
			AddRef();
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	ULONG __stdcall AddRef() override
	{
		return ++refCount;
	}

	ULONG __stdcall Release() override
	{
		long r = --refCount;
		if (r == 0)
			delete this;
		return r;
	}

	HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv) override
	{
		if (!ppv)
			return E_POINTER;

		*ppv = nullptr;

		if (pUnkOuter)
			return CLASS_E_NOAGGREGATION;

		auto* engine = new PhasorScriptEngine();
		HRESULT hr = engine->QueryInterface(riid, ppv);
		engine->Release();
		return hr;
	}

	HRESULT __stdcall LockServer(BOOL) override
	{
		return S_OK;
	}
};

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdll-attribute-on-redeclaration"
#endif

#if defined(_MSC_VER) && !defined(__clang__)
    #if defined(_M_IX86)
        #pragma comment(linker, "/EXPORT:DllGetClassObject=_DllGetClassObject@12,PRIVATE")
        #pragma comment(linker, "/EXPORT:DllCanUnloadNow=_DllCanUnloadNow@0,PRIVATE")
    #else
        #pragma comment(linker, "/EXPORT:DllGetClassObject,PRIVATE")
        #pragma comment(linker, "/EXPORT:DllCanUnloadNow,PRIVATE")
    #endif
    #define PHASOR_COM_EXPORT
#else
    #define PHASOR_COM_EXPORT __declspec(dllexport)
#endif

extern "C"
PHASOR_COM_EXPORT
HRESULT __stdcall DllGetClassObject(
	REFCLSID rclsid,
	REFIID riid,
	void** ppv)
{
	if (!ppv)
		return E_POINTER;

	*ppv = nullptr;

	if (!IsEqualCLSID(rclsid, CLSID_PhasorEngine))
		return CLASS_E_CLASSNOTAVAILABLE;

	auto* factory = new ClassFactory();
	HRESULT hr = factory->QueryInterface(riid, ppv);
	factory->Release();
	return hr;
}

extern "C"
PHASOR_COM_EXPORT
HRESULT __stdcall DllCanUnloadNow()
{
	return S_OK;
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif