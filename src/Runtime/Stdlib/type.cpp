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

i64 StdLib::to_int(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "to_int");
	if (args[0].isInt())
		return args[0].asInt();
	if (args[0].isFloat())
		return static_cast<i64>(args[0].asFloat());
	if (args[0].isString())
	{
		try
		{
			return static_cast<i64>(std::stoll(args[0].asString()));
		}
		catch (...)
		{
			return 0;
		}
	}
	if (args[0].isBool())
		return args[0].asBool() ? 1 : 0;
	return 0;
}

f64 StdLib::to_float(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "to_float");
	return args[0].asFloat();
}

PhsString StdLib::to_string(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "to_string");
	return args[0].toString();
}

PhsString StdLib::to_json(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "to_json", true);
	if (args.size() > 4) {
		throw std::runtime_error("to_json expects at most 4 arguments");
	}
	return args[0].jsonSerialize(args.size() > 1 ? static_cast<int>(args[1].asInt()) : -1, args.size() > 2 ? static_cast<int>(args[2].asInt()) : 0);
}

bool StdLib::to_bool(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "to_bool");
	return args[0].isTruthy();
}

Value StdLib::from_json(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "from_json");
	if (!args[0].isString())
		throw std::runtime_error("from_json expects a string argument");
	return Value::from_json(args[0].asString());
}

PhsString StdLib::ascii_to_string(const std::vector<Value> &args, VM *) {
	checkArgCount(args, 1, "ascii_to_string");
	if (!args[0].isInt())
		throw std::runtime_error("ascii_to_string expects an integer argument");
	return args[0].intToAscii();
}

Value StdLib::get_struct_elements(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 1, "get_elements");

    const auto &structVal = args[0];
    if (!structVal.isStruct())
    {
        throw std::runtime_error("meta_get_struct_elements: argument is not a struct");
    }

    const auto structPtr = structVal.asStruct();
    if (!structPtr)
    {
        throw std::runtime_error("meta_get_struct_elements: struct instance is null");
    }

    std::vector<Value> keys;
    keys.reserve(structPtr->fields.size());

    for (const auto &[key, _] : structPtr->fields)
    {
        keys.push_back(key);
    }

    return Value::createArray(std::move(keys));
}

Value StdLib::get_struct_elements_values(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "get_elements_values");

	const auto &structVal = args[0];
	if (!structVal.isStruct())
	{
		throw std::runtime_error("get_elements_values: argument is not a struct");
	}

	const auto structPtr = structVal.asStruct();
	if (!structPtr)
	{
		throw std::runtime_error("get_elements_values: struct instance is null");
	}

	std::vector<Value> values;
	values.reserve(structPtr->fields.size());

	for (const auto &[key, value] : structPtr->fields)
	{
		values.push_back(value);
	}

	return Value::createArray(std::move(values));
}

PhsString StdLib::get_type(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "get_type");
	auto type = args[0].getType();
	return Value::typeToString(type).asString();
}

} // namespace Phasor