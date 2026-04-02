#include "StdLib.hpp"
#include <cassert>

namespace Phasor
{

char **StdLib::argv = nullptr;
int    StdLib::argc = 0;
char **StdLib::envp = nullptr;

#ifndef __EMSCRIPTEN__
#ifndef SANDBOXED
int StdLib::dupenv(std::string &out, const char *name, char *const argp[])
{
	if (!name || !argp)
	{
		return 1;
	}

	const size_t key_len = strlen(name);

	const char *val = NULL;
	for (size_t i = 0; argp[i]; i++)
	{
		const char *entry = argp[i];
		if (strncmp(entry, name, key_len) == 0 && entry[key_len] == '=')
		{
			val = entry + key_len + 1;
			break;
		}
	}
	if (!val)
	{
		out.clear();
		return 2;
	}

	out = std::string(val);
	return 0;
}
#endif
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

Value StdLib::std_import(const std::vector<Value> &args, VM *vm)
{
#ifdef __EMSCRIPTEN__
	return false;
#else
	checkArgCount(args, 1, "using", true);
	for (const auto &arg : args)
	{
		if (arg.getType() != ValueType::String)
		{
			throw std::runtime_error("All arguments to 'using' must be strings");
			return false;
		}
		auto moduleName = arg.asString();
		
		if (moduleName == "stdio") [[likely]]
			registerIOFunctions(vm);
		else if (moduleName == "stdsys") [[likely]]
			registerSysFunctions(vm);
		else if (moduleName == "stdmath") 
			registerMathFunctions(vm);
		else if (moduleName == "stdstr") 
			registerStringFunctions(vm);
		else if (moduleName == "stdtype") 
			registerTypeConvFunctions(vm);
#ifndef SANDBOXED
		else if (moduleName == "stdfile")
			registerFileFunctions(vm);
#endif
		else if (moduleName == "std*") [[unlikely]]
		{	
			registerIOFunctions(vm);
			registerSysFunctions(vm);
			registerMathFunctions(vm);
			registerStringFunctions(vm);
			registerTypeConvFunctions(vm);
#ifndef SANDBOXED
			registerFileFunctions(vm);
#endif
		}
		else
		{
			throw std::runtime_error("Unknown standard library module: " + moduleName);
			return false;
		}
	}
	return true;
#endif
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