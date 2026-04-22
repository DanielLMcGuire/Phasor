#include "StdLib.hpp"

namespace Phasor
{

void StdLib::registerTypeConvFunctions(VM *vm)
{
	vm->registerNativeFunction("to_int", StdLib::to_int);
	vm->registerNativeFunction("to_float", StdLib::to_float);
	vm->registerNativeFunction("to_string", StdLib::to_string);
	vm->registerNativeFunction("to_bool", StdLib::to_bool);
}

int64_t StdLib::to_int(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "to_int");
	if (args[0].isInt())
		return args[0].asInt();
	if (args[0].isFloat())
		return static_cast<int64_t>(args[0].asFloat());
	if (args[0].isString())
	{
		try
		{
			return static_cast<int64_t>(std::stoll(args[0].asString()));
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

double StdLib::to_float(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "to_float");
	return args[0].asFloat();
}

std::string StdLib::to_string(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "to_string");
	return args[0].toString();
}

bool StdLib::to_bool(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "to_bool");
	return args[0].isTruthy();
}

} // namespace Phasor