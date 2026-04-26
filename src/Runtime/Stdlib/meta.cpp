#include "StdLib.hpp"
#include <version.h>

namespace Phasor
{

void StdLib::registerMetaFunctions(VM *vm)
{
#ifndef SANDBOXED
	vm->registerNativeFunction("phs_op", StdLib::meta_operation);
    vm->registerNativeFunction("phs_stack_run", StdLib::meta_stack_run);
#endif
    vm->registerNativeFunction("phs_version", StdLib::meta_get_version);
}

#ifndef SANDBOXED
int64_t StdLib::meta_operation(const std::vector<Value> &args, VM *vm) {
    checkArgCount(args, 1, "phs_op");
    if (args.size() > 4) throw std::runtime_error("Function 'phs_op' expects at most 4 arguments, but got " + std::to_string(args.size()));
    if (!args[0].isInt()) throw std::runtime_error("Function 'phs_op' expects an OpCode (int) as the first argument");

    auto ret = vm->operation(
        static_cast<Phasor::OpCode>(args[0].asInt()),
        args.size() > 1 && args[1].isInt() ? static_cast<int>(args[1].asInt()) : 0,
        args.size() > 2 && args[2].isInt() ? static_cast<int>(args[2].asInt()) : 0,
        args.size() > 3 && args[3].isInt() ? static_cast<int>(args[3].asInt()) : 0
    );
    return ret.isInt() ? ret.asInt() : 0;
}

Value StdLib::meta_stack_run(const std::vector<Value> &args, VM *vm) {
    checkArgCount(args, 1, "phs_stack_run");
    if (!args[0].isInt()) throw std::runtime_error("Function 'phs_stack_run' expects an OpCode (int) as the first argument");
    auto opcode = static_cast<Phasor::OpCode>(args[0].asInt());

    for (size_t i = args.size(); i-- > 1;)
    vm->push(args[i]);

    vm->operation(opcode);
    return vm->pop();
}
#endif

std::string StdLib::meta_get_version(const std::vector<Value> &args, VM *) {
    checkArgCount(args, 0, "phs_version");
    return PHASOR_VERSION_STRING;
}

} // namespace Phasor
