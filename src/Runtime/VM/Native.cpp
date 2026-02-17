#include "VM.hpp"

void Phasor::VM::registerNativeFunction(const std::string &name, NativeFunction fn)
{
	nativeFunctions[name] = fn;
}