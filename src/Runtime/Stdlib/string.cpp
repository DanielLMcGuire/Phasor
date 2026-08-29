// Copyright 2026 Daniel McGuire
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

#include "StdLib.hpp"
#include <algorithm>
#include <string>
#include <phsint.hpp>
#include <utility>

namespace Phasor
{

void StdLib::registerStringFunctions(VM *vm)
{
	vm->registerNativeFunction("find", StdLib::str_find);
	vm->registerNativeFunction("len", StdLib::str_len);
	vm->registerNativeFunction("char_at", StdLib::str_char_at);
	vm->registerNativeFunction("substr", StdLib::str_substr);
	vm->registerNativeFunction("concat", StdLib::str_concat);
	vm->registerNativeFunction("to_upper", StdLib::str_upper);
	vm->registerNativeFunction("to_lower", StdLib::str_lower);
	vm->registerNativeFunction("starts_with", StdLib::str_starts_with);
	vm->registerNativeFunction("ends_with", StdLib::str_ends_with);
	vm->registerNativeFunction("split", StdLib::str_split);

	vm->registerNativeFunction("sb_new", StdLib::sb_new);
	vm->registerNativeFunction("sb_prealloc", StdLib::sb_prealloc);
	vm->registerNativeFunction("sb_append", StdLib::sb_append);
	vm->registerNativeFunction("sb_to_string", StdLib::sb_to_string);
	vm->registerNativeFunction("sb_free", StdLib::sb_free);
	vm->registerNativeFunction("sb_clear", StdLib::sb_clear);
}

static std::vector<Phasor::string>& getSbPool()
{
	static std::vector<Phasor::string> pool;
	return pool;
}

static std::vector<size_t>& getSbFreeIndices()
{
	static std::vector<size_t> freeIndices;
	return freeIndices;
}

/// @brief Validate a StringBuilder handle and return its index, throwing a descriptive error otherwise.
static size_t checkSbHandle(const Value &handle, const char *fnName)
{
	StdLib::requireInt(handle, fnName, "stringbuilder handle");

	i64 idx = handle.asInt();
	if (idx < 0 || std::cmp_greater_equal(idx ,getSbPool().size()))
		PHS_ERROR(std::string(fnName) + "(): invalid StringBuilder handle");

	return static_cast<size_t>(idx);
}

Value StdLib::str_split(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "split");

	requireString(args[0], "split", "string");
	requireString(args[1], "split", "delim");

	Phasor::string s = args[0].string();
	Phasor::string delim = args[1].string();

	Value::ArrayInstance result;
	if (delim.empty())
	{
		result.emplace_back(s);
		return Value::createArray(std::move(result));
	}

	size_t start = 0;
	size_t end = s.find(delim);
	
	while (end != Phasor::string::npos)
	{
		result.emplace_back(s.substr(start, end - start));
		start = end + delim.length();
		end = s.find(delim, start);
	}

	result.emplace_back(s.substr(start));
	return Value::createArray(std::move(result));
}

i64 StdLib::str_find(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "find", true);

	if (args.size() > 4)
		PHS_ERROR("find() expects at most 4 arguments");

	requireString(args[0], "find", "string");
	requireString(args[1], "find", "substring");

	Phasor::string s = args[0].string();
	Phasor::string sub = args[1].string();
	size_t      pos;
	if (args.size() == 3)
	{
		requireInt(args[2], "find", "start");
		i64 start = args[2].asInt();
		pos = s.find(sub, start);
		if (pos != Phasor::string::npos)
		{
			return static_cast<i64>(pos);
		}
		return -1;
	}
	if (args.size() == 4)
	{
		if (!args[2].isInt())
			PHS_ERROR("find() expects an integer as its third argument (start)");
		if (!args[3].isInt())
			PHS_ERROR("find() expects an integer as its fourth argument (end)");
		i64 start = args[2].asInt();
		i64 end = args[3].asInt();
		pos = s.find(sub, start);
		if (pos != Phasor::string::npos && pos < static_cast<size_t>(end))
		{
			return static_cast<i64>(pos);
		}
		return -1;
	} else {
		pos = s.find(sub);
	}
	return pos != Phasor::string::npos ? static_cast<i64>(pos) : 0;
}

i64 StdLib::sb_new(const Value::ArrayInstance &args, VM *)
{
	StdLib::checkArgCount(args, 0, "sb_new");
	size_t idx;
	if (!getSbFreeIndices().empty())
	{
		idx = getSbFreeIndices().back();
		getSbFreeIndices().pop_back();
		getSbPool()[idx] = "";
	} else {
		idx = getSbPool().size();
		getSbPool().emplace_back("");
	}
	return static_cast<i64>(idx);
}

i64 StdLib::sb_append(const Value::ArrayInstance &args, VM *)
{
	StdLib::checkArgCount(args, 2, "sb_append");
	size_t idx = checkSbHandle(args[0], "sb_append");

	getSbPool()[idx] += args[1].toString();
	return args[0].asInt(); // Return handle for chaining
}

i64 StdLib::sb_prealloc(const Value::ArrayInstance &args, VM *)
{
    StdLib::checkArgCount(args, 2, "sb_prealloc");

    size_t idx = checkSbHandle(args[0], "sb_prealloc");

    if (!args[1].isInt())
        PHS_ERROR("sb_prealloc() expects an integer as its second argument (capacity)");

    i64 capacity = args[1].asInt();
    if (capacity > 0)
    {
        getSbPool()[idx].reserve(static_cast<size_t>(capacity));
    }

    return args[0].asInt(); // Return handle for chaining
}

Phasor::string StdLib::sb_to_string(const Value::ArrayInstance &args, VM *)
{
	StdLib::checkArgCount(args, 1, "sb_to_string");
	size_t idx = checkSbHandle(args[0], "sb_to_string");

	return getSbPool()[idx];
}

Phasor::string StdLib::sb_free(const Value::ArrayInstance &args, VM *)
{
	StdLib::checkArgCount(args, 1, "sb_free");
	size_t idx = checkSbHandle(args[0], "sb_free");

	Phasor::string value = getSbPool()[idx];
	getSbFreeIndices().push_back(idx);
	return value;
}

i64 StdLib::sb_clear(const Value::ArrayInstance &args, VM *)
{
	StdLib::checkArgCount(args, 1, "sb_clear");
	size_t idx = checkSbHandle(args[0], "sb_clear");

	getSbPool()[idx].clear();
	return args[0].asInt(); // Return handle for chaining
}

Value StdLib::str_char_at(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "char_at");
	requireString(args[0], "char_at", "string");
	requireInt(args[1], "char_at", "index");

	const Phasor::string &s = args[0].string();
	i64            idx = args[1].asInt();
	if (idx < 0 || std::cmp_greater_equal(idx ,s.length())) 
	{
		return "";
	}
	return Phasor::string(1, s[idx]);
}

Value StdLib::str_substr(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "substr", true);
	if (args.size() > 3)
	{
		PHS_ERROR("substr() expects 2 or 3 arguments");
	}
	requireString(args[0], "substr", "string");
	requireInt(args[1], "substr", "start");
	if (args.size() == 3) requireInt(args[2], "substr", "length");

	Phasor::string s = args[0].string();
	i64     start = args[1].asInt();
	i64     len = (i64)args.size() == 3 ? args[2].asInt() : (i64)s.length() - start;

	if (start < 0 || std::cmp_greater_equal(start ,s.length()))
	{
		return "";
	}

	return s.substr(start, len);
}

Phasor::string StdLib::str_concat(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "concat", true);
	Phasor::string result = "";
	for (const auto &arg : args)
	{
		result += arg.toString();
	}
	return result;
}

i64 StdLib::str_len(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "len");
	Phasor::string s = args[0].toString();
	return static_cast<i64>(s.length());
}

Phasor::string StdLib::str_upper(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "to_upper");
	requireString(args[0], "to_lower", "string");
	Phasor::string s = args[0].string();
	std::ranges::transform(s, s.begin(), ::toupper);
	return s;
}

Phasor::string StdLib::str_lower(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "to_lower");
	requireString(args[0], "to_lower", "string");
	
	Phasor::string s = args[0].string();
	std::ranges::transform(s, s.begin(), ::tolower);
	return s;
}

Value StdLib::str_starts_with(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "starts_with");
	requireString(args[0], "ends_with", "string");
	requireString(args[1], "starts_with", "prefix");

	std::string s = args[0].string();
	std::string prefix = args[1].string();
	if (s.length() >= prefix.length())
	{
		return s.starts_with(prefix);
	}
	return false;
}

Value StdLib::str_ends_with(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "ends_with");
	requireString(args[0], "ends_with", "string");
	requireString(args[1], "ends_with", "suffix");

	std::string s = args[0].string();
	std::string suffix = args[1].string();
	if (s.length() >= suffix.length())
	{
		return s.ends_with(suffix);
	}
	return false;
}
} // namespace Phasor
