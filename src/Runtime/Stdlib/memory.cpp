#include "StdLib.hpp"

namespace Phasor
{

void StdLib::registerMemoryFunctions(VM *vm)
{
	vm->registerNativeFunction("free", StdLib::var_free);
}

Value StdLib::var_free(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "free");

	const Value &arg = args[0];

	if (!arg.isString())
		throw std::runtime_error("free(): argument must be a string");

	vm->freeVariableByName(arg.asString());
	return phsnull;
}

} // namespace Phasor