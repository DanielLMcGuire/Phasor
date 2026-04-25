#include "StdLib.hpp"
#include <cassert>
#ifdef _WIN32
#include <stdlib.h>
#else
#include <cstdlib>
#endif

namespace Phasor
{

char **StdLib::argv = nullptr;
int    StdLib::argc = 0;

#ifndef SANDBOXED
int StdLib::dupenv(std::string &out, const char *name)
{
    if (!name || name[0] == '\0') 
    {
        return 1;
    }

#ifdef _WIN32
    char *buffer = nullptr;
    size_t len = 0;
    if (_dupenv_s(&buffer, &len, name) == 0 && buffer != nullptr) 
    {
        out = buffer;
        free(buffer);
        return 0;
    }
#else
    const char *val = std::getenv(name);
    if (val) 
    {
        out = val;
        return 0;
    }
#endif

    out.clear();
    return 2; // Not found
}
#endif

void StdLib::checkArgCount(const std::vector<Value> &args, size_t minimumArguments, const std::string &name,
                           bool allowMoreArguments)
{
	if (args.size() < minimumArguments)
	{
		throw std::runtime_error("Function '" + name + "' expects at least " + std::to_string(minimumArguments) +
		                         " arguments, but got " + std::to_string(args.size()));
	}
	if (!allowMoreArguments && args.size() > minimumArguments)
	{
		throw std::runtime_error("Function '" + name + "' expects exactly " + std::to_string(minimumArguments) +
		                         " arguments, but got " + std::to_string(args.size()));
	}
}

bool StdLib::std_import(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "using", true);
	
	std::unordered_map<std::string, std::function<void(Phasor::VM*)>>  modules {
		{"stdio", registerIOFunctions},
		{"stdsys", registerSysFunctions},
		{"stdmath", registerMathFunctions},
		{"stdstr", registerStringFunctions},
		{"stdtype", registerTypeConvFunctions},
	    {"stdmeta", registerMetaFunctions},
		{"stdmem", registerMemoryFunctions},
#ifndef SANDBOXED
		{"stdfile", registerFileFunctions},
#endif
		{"std*", [](Phasor::VM* vm) {
			registerIOFunctions(vm);
			registerSysFunctions(vm);
			registerMathFunctions(vm);
			registerStringFunctions(vm);
			registerTypeConvFunctions(vm);
		    registerMetaFunctions(vm);
			registerMemoryFunctions(vm);
#ifndef SANDBOXED
			registerFileFunctions(vm);
#endif
		}},
	};

	for (const auto &arg : args)
	{
		auto it = modules.find(arg.asString());
		if (it != modules.end())
		{
			it->second(vm);
		}
		else
		{
			throw std::runtime_error("Unknown module: " + arg.asString());
		}
	}
	return true;
}

#ifndef SANDBOXED
Value StdLib::std_assert(const std::vector<Value>& args, VM* vm)
{
	checkArgCount(args, 1, "assert");

#ifdef TRACING
	#ifndef NDEBUG
		vm->log(std::format("StdLib::{}({:T})\n", __func__, args[0]));
	#else
		vm->log(std::format("StdLib::{}({:T}): Assertion skipped (NDEBUG)\n", __func__, args[0]));
	#endif
	vm->flush();
#endif

#ifndef NDEBUG
	if (!args[0].isTruthy())
	{
		vm->logerr(std::format("StdLib::{}({:T}): Assertion failed!\n", __func__, args[0]));
		vm->flusherr();
	}
	assert(args[0].isTruthy());
#endif
	return Value();
}
#endif

} // namespace Phasor