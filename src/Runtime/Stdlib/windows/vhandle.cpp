#include "vhandle.hpp"
#include <winsock2.h>

namespace Phasor
{

namespace vhandle
{

std::unordered_map<VHANDLE, HandleEntry, GUIDHash, GUIDEqual> vHandleMap;

std::string vhandleGuid(const VHANDLE &vhandle)
{
	char buffer[39];
	std::snprintf(buffer, sizeof(buffer), "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", vhandle.Data1,
	              vhandle.Data2, vhandle.Data3, vhandle.Data4[0], vhandle.Data4[1], vhandle.Data4[2], vhandle.Data4[3],
	              vhandle.Data4[4], vhandle.Data4[5], vhandle.Data4[6], vhandle.Data4[7]);

	return std::string(buffer);
}

VHANDLE createVHandle(const void *handle, HandleType type, const bool isVMOwned)
{
	VHANDLE vhandle;
	if (CoCreateGuid(&vhandle) != S_OK)
	{
		throw std::runtime_error("Failed to create vhandle");
	}

	HandleEntry entry;
	entry.handle = const_cast<void *>(handle);
	entry.type = type;
	entry.isVMOwned = isVMOwned;

	vHandleMap[vhandle] = entry;

	return vhandle;
}

VHANDLE guidToVHandle(const std::string &guidStr)
{
	VHANDLE      vhandle = {};
	unsigned int data1, data2, data3;
	unsigned int b[8];

	int parsed = sscanf_s(guidStr.c_str(), "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", &data1, &data2, &data3,
	                      &b[0], &b[1], &b[2], &b[3], &b[4], &b[5], &b[6], &b[7]);

	if (parsed != 11)
	{
		throw std::runtime_error("Invalid GUID string format");
	}

	vhandle.Data1 = data1;
	vhandle.Data2 = static_cast<unsigned short>(data2);
	vhandle.Data3 = static_cast<unsigned short>(data3);

	for (int i = 0; i < 8; i++)
	{
		vhandle.Data4[i] = static_cast<unsigned char>(b[i]);
	}

	return vhandle;
}

template <typename T> T getHandle(const VHANDLE &vhandle)
{
	auto it = vHandleMap.find(vhandle);
	if (it != vHandleMap.end())
	{
		return reinterpret_cast<T>(it->second.handle);
	}
	return nullptr;
}

// Explicit template instantiations
template void                *getHandle<void *>(const VHANDLE &);
template HINSTANCE            getHandle<HINSTANCE>(const VHANDLE &);
template HMODULE              getHandle<HMODULE>(const VHANDLE &);
template HWND                 getHandle<HWND>(const VHANDLE &);
template SECURITY_DESCRIPTOR *getHandle<SECURITY_DESCRIPTOR *>(const VHANDLE &);
template SECURITY_ATTRIBUTES *getHandle<SECURITY_ATTRIBUTES *>(const VHANDLE &);
template FARPROC              getHandle<FARPROC>(const VHANDLE &);

void freeVHandle(const VHANDLE &vhandle)
{
	auto it = vHandleMap.find(vhandle);
	if (it == vHandleMap.end())
	{
		return;
	}

	HandleEntry &entry = it->second;

	if (entry.isVMOwned && entry.handle)
	{
		switch (entry.type)
		{
		case HandleType::Kernel:
			CloseHandle(reinterpret_cast<HANDLE>(entry.handle));
			break;

		case HandleType::Module:
			FreeLibrary(reinterpret_cast<HMODULE>(entry.handle));
			break;

		case HandleType::Window:
			DestroyWindow(reinterpret_cast<HWND>(entry.handle));
			break;

		case HandleType::Socket:
			closesocket(reinterpret_cast<SOCKET>(entry.handle));
			break;

		case HandleType::Com:
			static_cast<IUnknown *>(entry.handle)->Release();
			__fallthrough;

		case HandleType::Procedure:
		case HandleType::Generic:
			break;
		}
	}

	vHandleMap.erase(it);
}

} // namespace vhandle

} // namespace Phasor