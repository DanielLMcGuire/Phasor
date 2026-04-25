#include "StdLib.hpp"

namespace Phasor
{

void StdLib::registerMemoryFunctions(VM *vm)
{
	vm->registerNativeFunction("stdmem", StdLib::var_free);
}

Value StdLib::var_free(const std::vector<Value> &args, VM *vm)
{
    checkArgCount(args, 1, "stdmem");

    const Value &arg = args[0];

    if (arg.getType() != ValueType::String)
        throw std::runtime_error("stdmem(): argument must be a string");

    vm->freeVariableByName(arg.asString());
}

} // namespace Phasor