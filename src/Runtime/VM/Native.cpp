#ifndef CMAKE
#include "VM.hpp"
#endif

void Phasor::VM::registerNativeFunction(const std::string &name, NativeFunction fn)
{
#ifdef TRACING
	log(std::format("VM::{}(\"{}\")\n", __func__, name));
	flush();
#endif
	nativeFunctions[name] = fn;
}