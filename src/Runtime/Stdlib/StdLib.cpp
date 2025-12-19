#include "StdLib.hpp"

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

std::string StdLib::fixEscapeSequences(const std::string &s)
{
	std::string out;
	out.reserve(s.size());
	for (size_t i = 0; i < s.size(); ++i)
	{
		if (s[i] == '\\' && i + 1 < s.size())
		{
			char esc = s[i + 1];
			switch (esc)
			{
			case 'n':
				out.push_back('\n');
				break;
			case 't':
				out.push_back('\t');
				break;
			case 'r':
				out.push_back('\r');
				break;
			case '\\':
				out.push_back('\\');
				break;
			case '"':
				out.push_back('"');
				break;
			case '\'':
				out.push_back('\'');
				break;
			case '0':
				out.push_back('\0');
				break;
			default:
				out.push_back(esc);
				break;
			}
			++i; // skip the escaped character
		}
		else
		{
			out.push_back(s[i]);
		}
	}
	return out;
}

void StdLib::checkArgCount(const std::vector<Value> &args, size_t expected, const std::string &name, bool allowMore)
{
	if (args.size() < expected)
	{
		throw std::runtime_error("Function '" + name + "' expects at least " + std::to_string(expected) +
		                         " arguments, but got " + std::to_string(args.size()));
	}
	if (!allowMore && args.size() > expected)
	{
		throw std::runtime_error("Function '" + name + "' expects exactly " + std::to_string(expected) +
		                         " arguments, but got " + std::to_string(args.size()));
	}
}