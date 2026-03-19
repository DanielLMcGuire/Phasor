#ifndef CMAKE
#include "VM.hpp"
#endif

void Phasor::VM::registerNativeFunction(const std::string &name, NativeFunction fn)
{
#ifdef _DEBUG
	log(std::format("{}(\"{}\")\n", __func__, name));
	flush();
#endif
	nativeFunctions[name] = fn;
}