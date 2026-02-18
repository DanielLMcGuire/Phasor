#pragma once

#include <unordered_map>
#include <cstdint>
/// @brief Handle system for managed win32 resources
namespace HandleSystem
{
using HandleId = uint32_t;

inline HandleId &nextId()
{
	static HandleId id = 1;
	return id;
}

template <typename T> struct Table
{
	static std::unordered_map<HandleId, T> &idToHandle()
	{
		static std::unordered_map<HandleId, T> map;
		return map;
	}

	static std::unordered_map<T, HandleId> &handleToId()
	{
		static std::unordered_map<T, HandleId> map;
		return map;
	}
};

template <typename T> inline HandleId store(T handle)
{
	if (!handle)
		return 0;

	auto &h2i = Table<T>::handleToId();
	auto &i2h = Table<T>::idToHandle();

	auto it = h2i.find(handle);
	if (it != h2i.end())
		return it->second;

	HandleId id = nextId()++;
	h2i[handle] = id;
	i2h[id] = handle;
	return id;
}

template <typename T> inline T resolve(HandleId id)
{
	if (id == 0)
		return T{};

	auto &i2h = Table<T>::idToHandle();
	auto  it = i2h.find(id);
	if (it == i2h.end())
		return T{};

	return it->second;
}

template <typename T> inline void remove(HandleId id)
{
	auto &i2h = Table<T>::idToHandle();
	auto &h2i = Table<T>::handleToId();

	auto it = i2h.find(id);
	if (it == i2h.end())
		return;

	h2i.erase(it->second);
	i2h.erase(it);
}
} // namespace HandleSystem
