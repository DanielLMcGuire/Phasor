#include <Phasor.hpp>

int64_t phasor_test(const std::vector<Phasor::Value> &args, Phasor::VM *vm)
{
	Phasor::StdLib::checkArgCount(args, 0, "test");
	return 15 + 2;
}

int main(int argc, char **argv)
{
	Phasor::VM phasorrt;
	Phasor::StdLib::registerFunctions(phasorrt);
	phasorrt.registerNativeFunction("test", phasor_test);

	Phasor::Bytecode bc;
	bc.emit(Phasor::OpCode::CALL_NATIVE, bc.addConstant("test"));
	bc.emit(Phasor::OpCode::PRINT);
	bc.emit(Phasor::OpCode::HALT);
	return phasorrt.run(bc);
}