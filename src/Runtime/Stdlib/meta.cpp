#include "StdLib.hpp"

namespace Phasor
{

void StdLib::registerMetaFunctions(VM *vm)
{
	vm->registerNativeFunction("phs_op", StdLib::meta_operation);
    vm->registerNativeFunction("phs_stack_run", StdLib::meta_stack_run);
}

Value StdLib::meta_operation(const std::vector<Value> &args, VM *vm) {
    checkArgCount(args, 1, "phs_op");
    if (args.size() > 4) throw std::runtime_error("Function 'phs_op' expects at most 4 arguments, but got " + std::to_string(args.size()));
    if (!args[0].isInt()) throw std::runtime_error("Function 'phs_op' expects an OpCode (int) as the first argument");

    return vm->operation(
        static_cast<Phasor::OpCode>(args[0].asInt()),
        args.size() > 1 && args[1].isInt() ? args[1].asInt() : 0,
        args.size() > 2 && args[2].isInt() ? args[2].asInt() : 0,
        args.size() > 3 && args[3].isInt() ? args[3].asInt() : 0
    );
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

} // namespace Phasor