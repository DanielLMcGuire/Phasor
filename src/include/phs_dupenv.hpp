#pragma once
#include <cstdlib>
#include <PhasorString.hpp>

namespace Phasor
{

enum class dupenv_ret {
	Success = 0,
	InvalidInput = 1,
	NotFound = 2
};

inline dupenv_ret dupenv(PhsString &out, const char *name)
{
	if (!name || name[0] == '\0')
	{
		return dupenv_ret::InvalidInput;
	}

#ifdef _WIN32
	char  *buffer = nullptr;
	size_t len = 0;
	if (_dupenv_s(&buffer, &len, name) == 0 && buffer != nullptr)
	{
		out = buffer;
		free(buffer);
		return dupenv_ret::Success;
	}
#else
	const char *val = std::getenv(name);
	if (val)
	{
		out = val;
		return dupenv_ret::Success;
	}
#endif

	out.clear();
	return dupenv_ret::NotFound;
}

} // namespace Phasor
