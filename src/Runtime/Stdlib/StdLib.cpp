#include "StdLib.hpp"

namespace Phasor
{

char **StdLib::argv = nullptr;
int    StdLib::argc = 0;
char **StdLib::envp = nullptr;

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

void StdLib::checkArgCount(const std::vector<Value> &args, VM *vm, size_t minimumArguments, const std::string &name,
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
	checkArgCount(args, vm, 1, "using", true);
	for (const auto &arg : args)
	{
		if (arg.getType() != ValueType::String)
		{
			throw std::runtime_error("All arguments to 'using' must be strings");
			return false;
		}
		auto moduleName = arg.asString();
		if (moduleName == "stdio") registerIOFunctions(vm);
		else if (moduleName == "stdsys") registerSysFunctions(vm);
		else if (moduleName == "stdmath") registerMathFunctions(vm);
		else if (moduleName == "stdstr") registerStringFunctions(vm);
		else if (moduleName == "stdtype") registerTypeConvFunctions(vm);
		else if (moduleName == "stdfile") registerFileFunctions(vm);
		else
		{
			throw std::runtime_error("Unknown standard library module: " + moduleName);
			return false;
		}
	}
	return true;
}

} // namespace Phasor