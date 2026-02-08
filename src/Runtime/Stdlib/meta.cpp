#include "Runtime/Value.hpp"
#include "StdLib.hpp"



namespace Phasor
{

Value StdLib::meta_extr(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "pulsar_extr");
	return Value(meta_interp->getSymbol(args[0].asString()).value);
}

Value StdLib::meta_exec(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "pulsar_exec");
	meta_interp->runFromFile(args[0].asString());
	return true;
}

Value StdLib::meta_expr(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "pulsar");
	return Value(meta_interp->runCommand(args[0].asString()));
}

Value StdLib::meta_dbg(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "pulsar_dbg");
	meta_interp->DEBUG = (int)!(bool)meta_interp->DEBUG;
	return true;
}

Value StdLib::registerMetaFunctions(const std::vector<Value> &args, VM *vm)
{
	vm->registerNativeFunction("pulsar", StdLib::meta_expr);
	vm->registerNativeFunction("pulsar_exec", StdLib::meta_exec);
	vm->registerNativeFunction("pulsar_get", StdLib::meta_extr);
	vm->registerNativeFunction("pulsar_dbg", StdLib::meta_dbg);
	return true;
}
} // namespace Phasor