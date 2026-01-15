#include "StdLib.hpp"
#include <string>

Value StdLib::registerStringFunctions(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "include_stdstr");
	vm->registerNativeFunction("len", StdLib::str_len);
	vm->registerNativeFunction("char_at", StdLib::str_char_at);
	vm->registerNativeFunction("substr", StdLib::str_substr);
	vm->registerNativeFunction("concat", StdLib::str_concat);
	vm->registerNativeFunction("to_upper", StdLib::str_upper);
	vm->registerNativeFunction("to_lower", StdLib::str_lower);
	vm->registerNativeFunction("starts_with", StdLib::str_starts_with);
	vm->registerNativeFunction("ends_with", StdLib::str_ends_with);

	// StringBuilder
	vm->registerNativeFunction("sb_new", StdLib::sb_new);
	vm->registerNativeFunction("sb_append", StdLib::sb_append);
	vm->registerNativeFunction("sb_to_string", StdLib::sb_to_string);
	vm->registerNativeFunction("sb_free", StdLib::sb_free);
	return true;
}

// StringBuilder Pool
static std::vector<std::string> sbPool;
static std::vector<size_t>      sbFreeIndices;

Value StdLib::sb_new(const std::vector<Value> &args, VM *vm)
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
	return static_cast<int64_t>(idx);
}

Value StdLib::sb_append(const std::vector<Value> &args, VM *vm)
{
	StdLib::checkArgCount(args, 2, "sb_append");
	int64_t idx = args[0].asInt();
	if (idx < 0 || idx >= static_cast<int64_t>(sbPool.size()))
		throw std::runtime_error("Invalid StringBuilder handle");

	sbPool[idx] += args[1].toString();
	return args[0]; // Return handle for chaining
}

Value StdLib::sb_to_string(const std::vector<Value> &args, VM *vm)
{
	StdLib::checkArgCount(args, 1, "sb_to_string");
	int64_t idx = args[0].asInt();
	if (idx < 0 || idx >= static_cast<int64_t>(sbPool.size()))
		throw std::runtime_error("Invalid StringBuilder handle");

	return Value(sbPool[idx]);
}

Value StdLib::sb_free(const std::vector<Value> &args, VM *vm)
{
	StdLib::checkArgCount(args, 1, "sb_free");
	size_t idx = args[0].asInt();
	std::string value = sbPool[idx];
	sbFreeIndices.push_back(idx);
	return value;
}

Value StdLib::str_char_at(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 2, "char_at");
	if (args[0].isString())
	{
		const std::string &s = args[0].asString();
		int64_t            idx = args[1].asInt();
		if (idx < 0 || idx >= static_cast<int64_t>(s.length()))
			return Value("");
		return Value(std::string(1, s[idx]));
	}
	throw std::runtime_error("char_at() expects a string");
}

Value StdLib::str_substr(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 2, "substr", true);
	if (args.size() < 2 || args.size() > 3)
	{
		throw std::runtime_error("substr() expects 2 or 3 arguments");
	}
	std::string s = args[0].asString();
	int64_t     start = args[1].asInt();
	int64_t     len = args.size() == 3 ? args[2].asInt() : s.length() - start;

	if (start < 0 || start >= static_cast<int64_t>(s.length()))
	{
		return Value("");
	}

	return Value(s.substr(start, len));
}

Value StdLib::str_concat(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 2, "concat");
	std::string result = "";
	for (const auto &arg : args)
	{
		result += arg.toString();
	}
	return Value(result);
}

Value StdLib::str_len(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "len");
	if (!args[0].isString()) {
		throw std::runtime_error("len() expects a string argument");
	}
	return static_cast<int64_t>(args[0].asString().length());
}

Value StdLib::str_upper(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "to_upper");
	std::string s = args[0].asString();
	std::transform(s.begin(), s.end(), s.begin(), ::toupper);
	return Value(s);
}

Value StdLib::str_lower(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "to_lower");
	std::string s = args[0].asString();
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);
	return Value(s);
}

Value StdLib::str_starts_with(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 2, "starts_with");
	std::string s = args[0].asString();
	std::string prefix = args[1].asString();
	if (s.length() >= prefix.length())
	{
		return Value(s.compare(0, prefix.length(), prefix) == 0);
	}
	return Value(false);
}

Value StdLib::str_ends_with(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 2, "ends_with");
	std::string s = args[0].asString();
	std::string suffix = args[1].asString();
	if (s.length() >= suffix.length())
	{
		return Value(s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0);
	}
	return Value(false);
}