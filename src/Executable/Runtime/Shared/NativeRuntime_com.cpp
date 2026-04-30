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

HRESULT PhasorScriptEngine::QueryInterface(REFIID riid, void** ppv)
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

ULONG PhasorScriptEngine::AddRef()
{
	return ++refCount;
}

ULONG PhasorScriptEngine::Release()
{
	long r = --refCount;
	if (r == 0)
		delete this;
	return r;
}

HRESULT PhasorScriptEngine::SetScriptSite(IActiveScriptSite* pSite)
{
	if (pSite)
		pSite->AddRef();

	if (site)
		site->Release();

	site = pSite;
	return S_OK;
}

HRESULT PhasorScriptEngine::GetScriptSite(REFIID riid, void** ppvObject)
{
	if (!ppvObject)
		return E_POINTER;

	*ppvObject = nullptr;

	if (!site)
		return E_FAIL;

	return site->QueryInterface(riid, ppvObject);
}

HRESULT PhasorScriptEngine::SetScriptState(SCRIPTSTATE st)
{
	state = st;
	return S_OK;
}

HRESULT PhasorScriptEngine::GetScriptState(SCRIPTSTATE* pss)
{
	if (!pss)
		return E_POINTER;

	*pss = state;
	return S_OK;
}

HRESULT PhasorScriptEngine::Close()
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

HRESULT PhasorScriptEngine::AddNamedItem(LPCOLESTR, DWORD)
{
    return E_NOTIMPL;
}

HRESULT PhasorScriptEngine::AddTypeLib(REFGUID, DWORD, DWORD, DWORD)
{
	return E_NOTIMPL;
}

HRESULT PhasorScriptEngine::GetScriptDispatch(LPCOLESTR, IDispatch** ppdisp)
{
	if (!ppdisp)
		return E_POINTER;

	*ppdisp = nullptr;
	return E_NOTIMPL;
}

HRESULT PhasorScriptEngine::GetCurrentScriptThreadID(SCRIPTTHREADID* pstidThread)
{
	if (!pstidThread)
		return E_POINTER;

	*pstidThread = SCRIPTTHREADID_BASE;
	return S_OK;
}

HRESULT PhasorScriptEngine::GetScriptThreadID(DWORD, SCRIPTTHREADID* pstidThread)
{
	if (!pstidThread)
		return E_POINTER;

	*pstidThread = SCRIPTTHREADID_BASE;
	return S_OK;
}

HRESULT PhasorScriptEngine::GetScriptThreadState(SCRIPTTHREADID, SCRIPTTHREADSTATE* pstsState)
{
	if (!pstsState)
		return E_POINTER;

	*pstsState = SCRIPTTHREADSTATE_NOTINSCRIPT;
	return S_OK;
}

HRESULT PhasorScriptEngine::InterruptScriptThread(SCRIPTTHREADID, const EXCEPINFO*, DWORD)
{
	return E_NOTIMPL;
}

HRESULT PhasorScriptEngine::Clone(IActiveScript** ppscript)
{
	if (ppscript)
		*ppscript = nullptr;
	return E_NOTIMPL;
}

HRESULT PhasorScriptEngine::InitNew()
{
	vm.reset(true, true, true);
	Phasor::StdLib::registerFunctions(vm);
	state = SCRIPTSTATE_INITIALIZED;
	return S_OK;
}

HRESULT PhasorScriptEngine::AddScriptlet(
	LPCOLESTR,
	LPCOLESTR,
	LPCOLESTR,
	LPCOLESTR,
	LPCOLESTR,
	LPCOLESTR,
	DWORD_PTR,
	ULONG,
	DWORD,
	BSTR* pbstrName,
	EXCEPINFO*)
{
	if (pbstrName)
		*pbstrName = nullptr;

	return E_NOTIMPL;
}

HRESULT PhasorScriptEngine::ParseScriptText(
	LPCOLESTR code,
	LPCOLESTR,
	IUnknown*,
	LPCOLESTR,
	DWORD_PTR,
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
	HRESULT QueryInterface(REFIID riid, void** ppv) override
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

	ULONG AddRef() override
	{
		return ++refCount;
	}

	ULONG Release() override
	{
		long r = --refCount;
		if (r == 0)
			delete this;
		return r;
	}

	HRESULT CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv) override
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

	HRESULT LockServer(BOOL) override
	{
		return S_OK;
	}
};

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdll-attribute-on-redeclaration"
#endif

extern "C"
__declspec(dllexport)
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
__declspec(dllexport)
HRESULT __stdcall DllCanUnloadNow()
{
	return S_OK;
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif