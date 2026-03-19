#ifndef CMAKE
#include "VM.hpp"
#endif

void Phasor::VM::registerNativeFunction(const std::string &name, NativeFunction fn)
{
	nativeFunctions[name] = fn;
}