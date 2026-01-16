#include "winType.hpp"
#include <stdexcept>

namespace Phasor
{

namespace win
{

// ============================================================
// strings
// ============================================================

LPCSTR asLPCSTR(const Value &v)
{
	if (!v.isString())
		throw std::runtime_error("Expected string");
	static thread_local std::string str = v.asString();
	str = v.asString();
	return str.c_str();
}

LPCWSTR asLPCWSTR(const Value &v)
{
	if (!v.isString())
		throw std::runtime_error("Expected string");
	std::string str = v.asString();
	int         len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	wchar_t    *wstr = new wchar_t[len];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, wstr, len);
	return wstr;
}

LPCSTR asOptionalLPCSTR(const Value &v)
{
	if (v.isNull())
		return nullptr;
	return asLPCSTR(v);
}

LPCWSTR asOptionalLPCWSTR(const Value &v)
{
	if (v.isNull())
		return nullptr;
	return asLPCWSTR(v);
}

LPSTR asLPSTR(const Value &v, size_t &capacity)
{
	if (!v.isString())
		throw std::runtime_error("Expected string");
	std::string str = v.asString();
	capacity = str.length() + 1;
	LPSTR buffer = new char[capacity];
	strcpy_s(buffer, capacity, str.c_str());
	return buffer;
}

LPWSTR asLPWSTR(const Value &v, size_t &capacity)
{
	if (!v.isString())
		throw std::runtime_error("Expected string");
	std::string str = v.asString();
	int         len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	capacity = len;
	LPWSTR buffer = new wchar_t[capacity];
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer, len);
	return buffer;
}

// ============================================================
// value sized integers (32 bit)
// ============================================================

DWORD asDWORD(const Value &v)
{
	return static_cast<DWORD>(v.asInt());
}

UINT asUINT(const Value &v)
{
	return static_cast<UINT>(v.asInt());
}

WORD asWORD(const Value &v)
{
	return static_cast<WORD>(v.asInt());
}

BYTE asBYTE(const Value &v)
{
	return static_cast<BYTE>(v.asInt());
}

BOOL asBOOL(const Value &v)
{
	return v.asBool() ? TRUE : FALSE;
}

LONG asLONG(const Value &v)
{
	return static_cast<LONG>(v.asInt());
}

ULONG asULONG(const Value &v)
{
	return static_cast<ULONG>(v.asInt());
}

// ============================================================
// pointer sized integers
// ============================================================

LPARAM asLPARAM(const Value &v)
{
	return static_cast<LPARAM>(v.asInt());
}

WPARAM asWPARAM(const Value &v)
{
	return static_cast<WPARAM>(v.asInt());
}

LRESULT asLRESULT(const Value &v)
{
	return static_cast<LRESULT>(v.asInt());
}

intptr_t asINTPTR(const Value &v)
{
	if (v.isFloat())
		throw std::runtime_error("Expected integer, not float");
	return static_cast<intptr_t>(v.asInt());
}

uintptr_t asUINTPTR(const Value &v)
{
	if (v.isFloat())
		throw std::runtime_error("Expected integer, not float");
	return static_cast<uintptr_t>(v.asInt());
}

// ============================================================
// explicit 64 bit
// ============================================================

int64_t asINT64(const Value &v)
{
	return static_cast<int64_t>(v.asInt());
}

uint64_t asUINT64(const Value &v)
{
	return static_cast<uint64_t>(v.asInt());
}

// ============================================================
// handles and opaque pointers
// ============================================================

template <typename T> T asHandle(const Value &v)
{
	if (!v.isString())
		throw std::runtime_error("Expected GUID string");
	vhandle::VHANDLE handle = vhandle::guidToVHandle(v.asString());
	return vhandle::getHandle<T>(handle);
}

template <typename T> T asOptionalHandle(const Value &v)
{
	if (v.isNull())
		return nullptr;
	return asHandle<T>(v);
}

// Explicit template instantiations
template void                *asOptionalHandle<void *>(const Value &);
template SECURITY_DESCRIPTOR *asOptionalHandle<SECURITY_DESCRIPTOR *>(const Value &);
template SECURITY_ATTRIBUTES *asOptionalHandle<SECURITY_ATTRIBUTES *>(const Value &);

HANDLE asHANDLE(const Value &v)
{
	return asHandle<HANDLE>(v);
}

HWND asHWND(const Value &v)
{
	return asHandle<HWND>(v);
}

HMODULE asHMODULE(const Value &v)
{
	return asHandle<HMODULE>(v);
}

// ============================================================
// output conversions
// ============================================================

Value fromBOOL(BOOL v)
{
	return v != FALSE;
}

Value fromDWORD(DWORD v)
{
	return static_cast<int64_t>(v);
}

Value fromUINT(const UINT v)
{
	return static_cast<int64_t>(v);
}

Value fromINT64(int64_t v)
{
	return v;
}

Value fromUINT64(uint64_t v)
{
	return static_cast<int64_t>(v);
}

Value wrapKernelHandle(HANDLE h)
{
	return vhandle::vhandleGuid(vhandle::createVHandle(h, vhandle::HandleType::Kernel, true));
}

Value wrapUserHandle(HANDLE h)
{
	return vhandle::vhandleGuid(vhandle::createVHandle(h, vhandle::HandleType::Generic, true));
}

// ============================================================
// raw memory
// ============================================================

void *asBuffer(const Value &v, size_t &size)
{
	if (!v.isString())
		throw std::runtime_error("Expected string buffer");
	std::string str = v.asString();
	size = str.length();
	void *buffer = malloc(size);
	memcpy(buffer, str.c_str(), size);
	return buffer;
}

void *asOptionalBuffer(const Value &v, size_t &size)
{
	if (v.isNull())
	{
		size = 0;
		return nullptr;
	}
	return asBuffer(v, size);
}

} // namespace win
} // namespace Phasor