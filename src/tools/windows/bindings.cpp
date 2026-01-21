#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

struct Param
{
	std::string type;
	std::string name;
};

struct Function
{
	std::string        returnType;
	std::string        name;
	std::vector<Param> params;
	std::string        rawLine;
	int                lineNumber;
};

bool isSupportedParam(const std::string &type)
{
	std::string t = type;
	t.erase(std::remove(t.begin(), t.end(), ' '), t.end());
	return t == "BOOL" || t == "DWORD" || t == "int" || t == "LONG" || t == "UINT" || t == "ULONG" || t == "float" ||
	       t == "double" || t == "LPCSTR" || t == "LPSTR" || t == "constchar*" || t == "LPCWSTR" || t == "LPWSTR" ||
	       t == "constwchar_t*";
}

std::string mapReturnType(const std::string &type)
{
	if (type == "BOOL")
		return "phasor_make_bool";
	if (type == "DWORD" || type == "int" || type == "LONG" || type == "UINT" || type == "ULONG")
		return "phasor_make_int";
	if (type == "float" || type == "double")
		return "phasor_make_float";
	if (type == "LPCSTR" || type == "LPSTR" || type == "constchar*" || type == "LPCWSTR" || type == "LPWSTR" ||
	    type == "constwchar_t*")
		return "phasor_make_string";
	return "";
}

std::string trim(const std::string &str)
{
	size_t start = str.find_first_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	size_t end = str.find_last_not_of(" \t\r\n");
	return str.substr(start, end - start + 1);
}

Function parseFunction(const std::string &line, int lineNumber)
{
	Function f;
	f.rawLine = line;
	f.lineNumber = lineNumber;

	std::string clean = line;
	clean.erase(std::remove(clean.begin(), clean.end(), ';'), clean.end());

	size_t paren = clean.find('(');
	if (paren == std::string::npos)
		return f;
	std::string retAndName = clean.substr(0, paren);
	std::string paramStr = clean.substr(paren + 1, clean.find(')') - paren - 1);

	std::istringstream iss(retAndName);
	iss >> f.returnType >> f.name;

	// Handle void or empty parameters
	paramStr = trim(paramStr);
	if (paramStr.empty() || paramStr == "void")
	{
		// No parameters - this is valid
		return f;
	}

	std::istringstream pstream(paramStr);
	std::string        param;
	while (std::getline(pstream, param, ','))
	{
		param = trim(param);
		if (param.empty())
			continue;

		std::istringstream ps(param);
		Param              p;
		ps >> p.type >> p.name;

		if (!isSupportedParam(p.type))
		{
			f.returnType = ""; // skip unsupported
			break;
		}
		f.params.push_back(p);
	}

	return f;
}

void generateWrapper(const Function &f, std::ostream &out)
{
	if (f.returnType.empty())
		return;

	out << "// " << f.rawLine << " @ln:" << f.lineNumber << "\n";
	out << "static PhasorValue win32_" << f.name << "(PhasorVM* vm, int argc, const PhasorValue* argv) {\n";

	for (size_t i = 0; i < f.params.size(); ++i)
	{
		const auto &p = f.params[i];
		std::string conv;
		if (p.type == "BOOL")
			conv = "phasor_to_bool(argv[" + std::to_string(i) + "])";
		else if (p.type == "DWORD" || p.type == "int" || p.type == "LONG" || p.type == "UINT" || p.type == "ULONG")
			conv = "(" + p.type + ")phasor_to_int(argv[" + std::to_string(i) + "])";
		else if (p.type == "float" || p.type == "double")
			conv = "(" + p.type + ")phasor_to_float(argv[" + std::to_string(i) + "])";
		else if (p.type == "LPCSTR" || p.type == "constchar*")
			conv = "(const char*)phasor_to_string(argv[" + std::to_string(i) + "])";
		else if (p.type == "LPSTR")
			conv = "(char*)phasor_to_string(argv[" + std::to_string(i) + "])";
		else if (p.type == "LPCWSTR" || p.type == "constwchar_t*")
			conv = "(const wchar_t*)phasor_to_string(argv[" + std::to_string(i) + "])";
		else // LPWSTR
			conv = "(wchar_t*)phasor_to_string(argv[" + std::to_string(i) + "])";
		out << "    " << p.type << " " << p.name << " = " << conv << ";\n";
	}

	out << "    auto result = " << f.name << "(";
	for (size_t i = 0; i < f.params.size(); ++i)
	{
		if (i > 0)
			out << ", ";
		out << f.params[i].name;
	}
	out << ");\n";

	std::string wrapFunc = mapReturnType(f.returnType);
	if (wrapFunc.empty())
		out << "    return phasor_make_null();\n";
	else
		out << "    return " << wrapFunc << "(result);\n";

	out << "}\n\n";
}

int main(int argc, char **argv)
{
	std::string inputFile = "winapi.txt";
	std::string outputFile = "phasor_winapi.cpp";
	if (argc >= 2)
		inputFile = argv[1];
	if (argc >= 4 && std::string(argv[2]) == "-o")
		outputFile = argv[3];

	std::ifstream infile(inputFile);
	std::ofstream outfile(outputFile);

	outfile << "#define PHASOR_FFI_BUILD_DLL\n#include <PhasorFFI.hpp>\n#include <windows.h>\n\n";

	std::string           line;
	int                   lineNumber = 0;
	std::vector<Function> funcs;
	while (std::getline(infile, line))
	{
		lineNumber++;
		if (line.empty())
			continue;
		Function f = parseFunction(line, lineNumber);
		if (!f.returnType.empty())
			funcs.push_back(f);
	}

	for (const auto &f : funcs)
		generateWrapper(f, outfile);

	outfile << "PHASOR_FFI_EXPORT void phasor_plugin_entry(const PhasorAPI* api, PhasorVM* vm) {\n";
	for (const auto &f : funcs)
	{
		outfile << "    api->register_function(vm, \"" << f.name << "\", win32_" << f.name << ");\n";
	}
	outfile << "}\n";
}