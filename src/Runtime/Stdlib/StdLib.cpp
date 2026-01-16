#include "StdLib.hpp"

#ifdef _WIN32
#include "windows/windows.hpp"
#endif

namespace Phasor
{

char **StdLib::argv = nullptr;
int    StdLib::argc = 0;
char **StdLib::envp = nullptr;

void StdLib::registerFunctions(VM &vm)
{
	vm.registerNativeFunction("include_stdio", registerIOFunctions);
	vm.registerNativeFunction("include_stdsys", registerSysFunctions);
	vm.registerNativeFunction("include_stdmath", registerMathFunctions);
	vm.registerNativeFunction("include_stdstr", registerStringFunctions);
	vm.registerNativeFunction("include_stdtype", registerTypeConvFunctions);
	vm.registerNativeFunction("include_stdfile", registerFileFunctions);
	vm.registerNativeFunction("include_stdregex", registerRegexFunctions);
#ifdef _WIN32
	vm.registerNativeFunction("include_win32api", windows::registerFunctions);
#endif
}

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

} // namespace Phasor