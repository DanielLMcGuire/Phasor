#include "StdLib.hpp"
#include <string>
#include <phsint.hpp>

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

static std::vector<PhsString>& getSbPool()
{
	static std::vector<PhsString> pool;
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
	if (!handle.isInt())
		PHS_ERROR(std::string(fnName) + "() expects an integer StringBuilder handle as its first argument");

	i64 idx = handle.asInt();
	if (idx < 0 || idx >= static_cast<i64>(getSbPool().size()))
		PHS_ERROR(std::string(fnName) + "(): invalid StringBuilder handle");

	return static_cast<size_t>(idx);
}

Value StdLib::str_split(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "split");

	if (!args[0].isString())
		PHS_ERROR("split() expects a string as its first argument");
	if (!args[1].isString())
		PHS_ERROR("split() expects a string as its second argument (delimiter)");

	PhsString s = args[0].string();
	PhsString delim = args[1].string();

	std::vector<Value> result;
	if (delim.empty()) {
		result.push_back(s);
		return Value::createArray(std::move(result));
	}

	size_t start = 0;
	size_t end = s.find(delim);
	
	while (end != PhsString::npos) {
		result.push_back(s.substr(start, end - start));
		start = end + delim.length();
		end = s.find(delim, start);
	}

	result.push_back(s.substr(start));
	return Value::createArray(std::move(result));
}

i64 StdLib::str_find(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "find", true);

	if (args.size() > 4)
		PHS_ERROR("find() expects at most 4 arguments");

	if (!args[0].isString())
		PHS_ERROR("find() expects a string as its first argument");
	if (!args[1].isString())
		PHS_ERROR("find() expects a string as its second argument (substring)");

	PhsString s = args[0].string();
	PhsString sub = args[1].string();
	size_t      pos;
	if (args.size() == 3)
	{
		if (!args[2].isInt())
			PHS_ERROR("find() expects an integer as its third argument (start)");
		i64 start = args[2].asInt();
		pos = s.find(sub, start);
		if (pos != PhsString::npos)
		{
			return static_cast<i64>(pos);
		}
		return -1;
	}
	else if (args.size() == 4)
	{
		if (!args[2].isInt())
			PHS_ERROR("find() expects an integer as its third argument (start)");
		if (!args[3].isInt())
			PHS_ERROR("find() expects an integer as its fourth argument (end)");
		i64 start = args[2].asInt();
		i64 end = args[3].asInt();
		pos = s.find(sub, start);
		if (pos != PhsString::npos && pos < static_cast<size_t>(end))
		{
			return static_cast<i64>(pos);
		}
		return -1;
	}
	else
	{
		pos = s.find(sub);
	}
	return pos != PhsString::npos ? static_cast<i64>(pos) : false;
}

i64 StdLib::sb_new(const std::vector<Value> &args, VM *)
{
	StdLib::checkArgCount(args, 0, "sb_new");
	size_t idx;
	if (!getSbFreeIndices().empty())
	{
		idx = getSbFreeIndices().back();
		getSbFreeIndices().pop_back();
		getSbPool()[idx] = "";
	}
	else
	{
		idx = getSbPool().size();
		getSbPool().push_back("");
	}
	return static_cast<i64>(idx);
}

i64 StdLib::sb_append(const std::vector<Value> &args, VM *)
{
	StdLib::checkArgCount(args, 2, "sb_append");
	size_t idx = checkSbHandle(args[0], "sb_append");

	getSbPool()[idx] += args[1].toString();
	return args[0].asInt(); // Return handle for chaining
}

i64 StdLib::sb_prealloc(const std::vector<Value> &args, VM *)
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

PhsString StdLib::sb_to_string(const std::vector<Value> &args, VM *)
{
	StdLib::checkArgCount(args, 1, "sb_to_string");
	size_t idx = checkSbHandle(args[0], "sb_to_string");

	return getSbPool()[idx];
}

PhsString StdLib::sb_free(const std::vector<Value> &args, VM *)
{
	StdLib::checkArgCount(args, 1, "sb_free");
	size_t idx = checkSbHandle(args[0], "sb_free");

	PhsString value = getSbPool()[idx];
	getSbFreeIndices().push_back(idx);
	return value;
}

i64 StdLib::sb_clear(const std::vector<Value> &args, VM *)
{
	StdLib::checkArgCount(args, 1, "sb_clear");
	size_t idx = checkSbHandle(args[0], "sb_clear");

	getSbPool()[idx].clear();
	return args[0].asInt(); // Return handle for chaining
}

Value StdLib::str_char_at(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "char_at");
	if (!args[0].isString())
		PHS_ERROR("char_at() expects a string as its first argument");
	if (!args[1].isInt())
		PHS_ERROR("char_at() expects an integer as its second argument (index)");

	const PhsString &s = args[0].string();
	i64            idx = args[1].asInt();
	if (idx < 0 || idx >= static_cast<i64>(s.length()))
		return Value("");
	return Value(PhsString(1, s[idx]));
}

Value StdLib::str_substr(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "substr", true);
	if (args.size() > 3)
	{
		PHS_ERROR("substr() expects 2 or 3 arguments");
	}

	if (!args[0].isString())
		PHS_ERROR("substr() expects a string as its first argument");
	if (!args[1].isInt())
		PHS_ERROR("substr() expects an integer as its second argument (start)");
	if (args.size() == 3 && !args[2].isInt())
		PHS_ERROR("substr() expects an integer as its third argument (length)");

	PhsString s = args[0].string();
	i64     start = args[1].asInt();
	i64     len = (i64)args.size() == 3 ? args[2].asInt() : (i64)s.length() - start;

	if (start < 0 || start >= static_cast<i64>(s.length()))
	{
		return Value("");
	}

	return Value(s.substr(start, len));
}

PhsString StdLib::str_concat(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "concat", true);
	PhsString result = "";
	for (const auto &arg : args)
	{
		result += arg.toString();
	}
	return result;
}

i64 StdLib::str_len(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "len");
	PhsString s = args[0].toString();
	return static_cast<i64>(s.length());
}

PhsString StdLib::str_upper(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "to_upper");
	if (!args[0].isString())
		PHS_ERROR("to_upper() expects a string as its argument");
	PhsString s = args[0].string();
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s;
}

PhsString StdLib::str_lower(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "to_lower");
	if (!args[0].isString())
		PHS_ERROR("to_lower() expects a string as its argument");
	PhsString s = args[0].string();
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

Value StdLib::str_starts_with(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "starts_with");
	if (!args[0].isString())
		PHS_ERROR("starts_with() expects a string as its first argument");
	if (!args[1].isString())
		PHS_ERROR("starts_with() expects a string as its second argument (prefix)");

	std::string s = args[0].string();
	std::string prefix = args[1].string();
	if (s.length() >= prefix.length())
	{
		return Value(s.compare(0, prefix.length(), prefix) == 0);
	}
	return Value(false);
}

Value StdLib::str_ends_with(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "ends_with");
	if (!args[0].isString())
		PHS_ERROR("ends_with() expects a string as its first argument");
	if (!args[1].isString())
		PHS_ERROR("ends_with() expects a string as its second argument (suffix)");

	std::string s = args[0].string();
	std::string suffix = args[1].string();
	if (s.length() >= suffix.length())
	{
		return Value(s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0);
	}
	return Value(false);
}
} // namespace Phasor
