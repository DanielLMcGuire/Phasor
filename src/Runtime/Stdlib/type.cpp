#include "StdLib.hpp"
#include <phsint.hpp>

namespace Phasor
{

void StdLib::registerTypeConvFunctions(VM *vm)
{
	vm->registerNativeFunction("to_int", StdLib::to_int);
	vm->registerNativeFunction("to_float", StdLib::to_float);
	vm->registerNativeFunction("to_string", StdLib::to_string);
	vm->registerNativeFunction("to_bool", StdLib::to_bool);
	vm->registerNativeFunction("to_json", StdLib::to_json);
	vm->registerNativeFunction("from_json", StdLib::from_json);
	vm->registerNativeFunction("ascii_to_string", StdLib::ascii_to_string);
	vm->registerNativeFunction("get_elements", StdLib::get_struct_elements);
	vm->registerNativeFunction("get_elements_values", StdLib::get_struct_elements_values);
	vm->registerNativeFunction("get_type", StdLib::get_type);
}

i64 StdLib::to_int(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "to_int");
	if (args[0].isInt()) 
	{
		return args[0].asInt();
	} else if (args[0].isFloat()) {
		return static_cast<i64>(args[0].asFloat());
	} else if (args[0].isString()) {
		try
		{
			return static_cast<i64>(std::stoll(args[0].string()));
		}
		catch (...)
		{
			return 0;
		}
	} else if (args[0].isBool()) {
		return args[0].asBool() ? 1 : 0;
	}
	return 0;
}

f64 StdLib::to_float(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "to_float");
	return args[0].asFloat();
}

Phasor::string StdLib::to_string(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "to_string");
	return args[0].toString();
}

Phasor::string StdLib::to_json(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "to_json", true);
	if (args.size() > 4)
	{
		PHS_ERROR("to_json() expects at most 4 arguments");
	}
	if (args.size() > 1) requireInt(args[0], "to_json", "indent");
	if (args.size() > 2) requireInt(args[1], "to_json", "depth");
	return args[0].jsonSerialize(args.size() > 1 ? static_cast<int>(args[1].asInt()) : -1, args.size() > 2 ? static_cast<int>(args[2].asInt()) : 0);
}

bool StdLib::to_bool(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "to_bool");
	return args[0].isTruthy();
}

Value StdLib::from_json(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "from_json");
	requireString(args[0], "from_json", "json data");
	return Value::from_json(args[0].string());
}

Phasor::string StdLib::ascii_to_string(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "ascii_to_string");
	requireInt(args[0], "ascii_to_string", "integer");
	return args[0].intToAscii();
}

Value StdLib::get_struct_elements(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 1, "get_elements");
	requireStruct(args[0], "get_elements", "struct");

	const auto structPtr = args[0].asStruct();
	if (!structPtr)
	{
		PHS_ERROR("get_elements_values: struct instance is null");
	}

    Value::ArrayInstance keys;
    keys.reserve(structPtr->fields.size());

    for (const auto &[key, _] : structPtr->fields)
    {
        keys.emplace_back(key);
    }

    return Value::createArray(std::move(keys));
}

Value StdLib::get_struct_elements_values(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "get_elements_values");
	requireStruct(args[0], "get_elements_values", "struct");

	const auto structPtr = args[0].asStruct();
	if (!structPtr)
	{
		PHS_ERROR("get_elements_values: struct instance is null");
	}

	Value::ArrayInstance values;
	values.reserve(structPtr->fields.size());

	for (const auto &[key, value] : structPtr->fields)
	{
		values.push_back(value);
	}

	return Value::createArray(std::move(values));
}

i64 StdLib::get_type(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "get_type");
	auto type = args[0].getType();
	return static_cast<i64>(type);
}

} // namespace Phasor