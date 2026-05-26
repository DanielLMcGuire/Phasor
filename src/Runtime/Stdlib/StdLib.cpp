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

StdLib::dupenv_ret StdLib::dupenv(PhsString &out, const char *name)
{
	if (!name || name[0] == '\0')
	{
		return dupenv_ret::InvalidInput;
	}

#ifdef _WIN32
	char  *buffer = nullptr;
	size_t len = 0;
	if (_dupenv_s(&buffer, &len, name) == 0 && buffer != nullptr)
	{
		out = buffer;
		free(buffer);
		return dupenv_ret::Success;
	}
#else
	const char *val = std::getenv(name);
	if (val)
	{
		out = val;
		return dupenv_ret::Success;
	}
#endif

	out.clear();
	return dupenv_ret::NotFound;
}

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

	std::unordered_map<PhsString, std::function<void(Phasor::VM *)>> modules{
	    {"stdio", registerIOFunctions},
	    {"stdsys", registerSysFunctions},
	    {"stdmath", registerMathFunctions},
	    {"stdstr", registerStringFunctions},
	    {"stdtype", registerTypeConvFunctions},
	    {"stdmeta", registerMetaFunctions},
	    {"stdmem", registerMemoryFunctions},
	    {"stdrand", registerRandomFunctions},
		{"stdarray", registerArrayFunctions},
#ifndef SANDBOXED
	    {"stdfile", registerFileFunctions},
#endif
	    {"std*",
	     [](Phasor::VM *vm) {
		     registerIOFunctions(vm);
		     registerSysFunctions(vm);
		     registerMathFunctions(vm);
		     registerStringFunctions(vm);
		     registerTypeConvFunctions(vm);
		     registerMetaFunctions(vm);
		     registerMemoryFunctions(vm);
		     registerRandomFunctions(vm);
			 registerArrayFunctions(vm);
#ifndef SANDBOXED
		     registerFileFunctions(vm);
#endif
	     }},
	};

	for (const auto &arg : args)
	{
		auto it = modules.find(arg.string());
		if (it != modules.end())
		{
			it->second(vm);
		}
		else
		{
			throw std::runtime_error("Unknown module: " + arg.string());
		}
	}
	return true;
}

#ifndef SANDBOXED
Value StdLib::std_assert(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "assert", true);

	if (args.size() > 2)
	{ [[unlikely]]
		throw std::runtime_error("Assert expects 1 or 2 arguments, but got " + std::to_string(args.size()));
	}

	bool haveMessage = false;
	const char* message = nullptr;

	if (args.size() == 2)
	{
		message = args[1].c_str();
		haveMessage = true;
	}

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
	{ [[unlikely]]
		vm->logerr(std::format("StdLib::{}({:T}): Assertion failed!\n", __func__, args[0]));
		if (haveMessage) vm->logerr(std::format("{}\n", message));
		vm->flusherr();
	}
	if (haveMessage) assert(args[0].isTruthy() && message);
	else assert(args[0].isTruthy());
#endif
	return phsnull;
}
#endif

} // namespace Phasor