// Copyright 2025-2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://phasor.pages.dev/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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

inline dupenv_ret dupenv(Phasor::string &out, const char *name)
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
