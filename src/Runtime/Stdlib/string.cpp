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

	vm->registerNativeFunction("sb_new", StdLib::sb_new);
	vm->registerNativeFunction("sb_append", StdLib::sb_append);
	vm->registerNativeFunction("sb_to_string", StdLib::sb_to_string);
	vm->registerNativeFunction("sb_free", StdLib::sb_free);
	vm->registerNativeFunction("sb_clear", StdLib::sb_clear);
}

// StringBuilder Pool
static std::vector<PhsString> sbPool;
static std::vector<size_t>      sbFreeIndices;

i64 StdLib::str_find(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "find", true);
	PhsString s = args[0].asString();
	PhsString sub = args[1].asString();
	size_t      pos;
	if (args.size() == 3)
	{
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
	if (!sbFreeIndices.empty())
	{
		idx = sbFreeIndices.back();
		sbFreeIndices.pop_back();
		sbPool[idx] = "";
	}
	else
	{
		idx = sbPool.size();
		sbPool.push_back("");
	}
	return static_cast<i64>(idx);
}

Value StdLib::sb_append(const std::vector<Value> &args, VM *)
{
	StdLib::checkArgCount(args, 2, "sb_append");
	i64 idx = args[0].asInt();
	if (idx < 0 || idx >= static_cast<i64>(sbPool.size()))
		throw std::runtime_error("Invalid StringBuilder handle");

	sbPool[idx] += args[1].toString();
	return args[0]; // Return handle for chaining
}

PhsString StdLib::sb_to_string(const std::vector<Value> &args, VM *)
{
	StdLib::checkArgCount(args, 1, "sb_to_string");
	i64 idx = args[0].asInt();
	if (idx < 0 || idx >= static_cast<i64>(sbPool.size()))
		throw std::runtime_error("Invalid StringBuilder handle");

	return sbPool[idx];
}

PhsString StdLib::sb_free(const std::vector<Value> &args, VM *)
{
	StdLib::checkArgCount(args, 1, "sb_free");
	size_t      idx = args[0].asInt();
	PhsString value = sbPool[idx];
	sbFreeIndices.push_back(idx);
	return value;
}

Value StdLib::sb_clear(const std::vector<Value> &args, VM *)
{
	StdLib::checkArgCount(args, 1, "sb_clear");
	size_t idx = args[0].asInt();
	if (idx >= sbPool.size())
		throw std::runtime_error("Invalid StringBuilder handle");
	sbPool[idx].clear();
	return args[0]; // Return handle for chaining
}

Value StdLib::str_char_at(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "char_at");
	if (args[0].isString())
	{
		const PhsString &s = args[0].asString();
		i64            idx = args[1].asInt();
		if (idx < 0 || idx >= static_cast<i64>(s.length()))
			return Value("");
		return Value(PhsString(1, s[idx]));
	}
	throw std::runtime_error("char_at() expects a string");
}

Value StdLib::str_substr(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "substr", true);
	if (args.size() > 3)
	{
		throw std::runtime_error("substr() expects 2 or 3 arguments");
	}
	PhsString s = args[0].asString();
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
	PhsString s = args[0].asString();
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	return s;
}

PhsString StdLib::str_lower(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "to_lower");
	PhsString s = args[0].asString();
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	return s;
}

Value StdLib::str_starts_with(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "starts_with");
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
	std::string s = args[0].string();
	std::string suffix = args[1].string();
	if (s.length() >= suffix.length())
	{
		return Value(s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0);
	}
	return Value(false);
}
} // namespace Phasor