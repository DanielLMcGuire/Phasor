#include "ScriptingRuntime.hpp"
#include "../../Backend/Lexer/Lexer.hpp"
#include "../../Backend/Parser/Parser.hpp"
#include "../../Codegen/CodeGen.hpp"
#include "../../Frontend/Frontend.hpp"
#include "../../Runtime/Stdlib/StdLib.hpp"
#include "../../Runtime/VM/VM.hpp"
#include "../../Runtime/FFI/ffi.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#define error(msg) MessageBoxA(NULL, std::string(msg).c_str(), "Phasor Runtime Error", MB_OK | MB_ICONERROR)
#else
#define error(msg) std::cerr << "Error: " << msg << std::endl
#endif

namespace Phasor
{

ScriptingRuntime::ScriptingRuntime(int argc, char *argv[], char *envp[])
{
	m_args.envp = envp;
	parseArguments(argc, argv);
}

int ScriptingRuntime::run()
{
	if (m_args.inputFile.empty())
		return 0;

	return runSource();
}

int ScriptingRuntime::runSource()
{
	std::ifstream file(m_args.inputFile);
	if (!file.is_open())
	{
		std::cerr << "Could not open file: " << m_args.inputFile << "\n";
		return 1;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string source = buffer.str();

	try
	{
		auto vm = createVm();
		runSourceString(source, *vm);
	}
	catch (const std::exception &e)
	{
		std::string errorMsg = std::string(e.what()) + "\n";
		error(errorMsg);
		return 1;
	}

	return 0;
}

void ScriptingRuntime::runSourceString(const std::string &source, VM &vm)
{
	Lexer  lexer(source);
	auto   tokens = lexer.tokenize();
	Parser parser(tokens);
	auto   program = parser.parse();

	if (m_args.verbose)
	{
		std::cout << "AST:\n";
		program->print();
		std::cout << "\n";
	}

	CodeGenerator codegen;
	auto          bytecode = codegen.generate(*program);

	vm.run(bytecode);
}

std::unique_ptr<VM> ScriptingRuntime::createVm()
{
	auto vm = std::make_unique<VM>();
	StdLib::registerFunctions(*vm);
	StdLib::argv = m_args.scriptArgv;
	StdLib::argc = m_args.scriptArgc;
	StdLib::envp = m_args.envp;

	FFI ffi("plugins", vm.get());

	vm->setImportHandler([this, vm_ptr = vm.get()](const std::filesystem::path &path) {
		std::ifstream file(path);
		if (!file.is_open())
			throw std::runtime_error("Could not open imported file: " + path.string());
		std::stringstream buffer;
		buffer << file.rdbuf();
		runSourceString(buffer.str(), *vm_ptr);
	});

	return vm;
}

void ScriptingRuntime::parseArguments(int argc, char *argv[])
{
	int defaultArgLocation = 1;
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];

		if (arg == "-v" || arg == "--verbose")
		{
			m_args.verbose = true;
		}
		else if (arg == "-h" || arg == "--help")
		{
			showHelp(argv[0]);
			exit(0);
		}
		else
		{
			defaultArgLocation = i;
			m_args.inputFile = arg;
			break; // Stop parsing after finding the input file
		}
	}
	m_args.scriptArgv = argv + defaultArgLocation;
	m_args.scriptArgc = argc - defaultArgLocation;
}

void ScriptingRuntime::showHelp(const std::string &programName)
{
	std::string filename = std::filesystem::path(programName).filename().string();
	std::cout << "Phasor Scripting Runtime\n\n";
	std::cout << "Usage:\n";
	std::cout << "  " << filename << " [options] [file.phs] [...script args]\n\n";
	std::cout << "Options:\n";
	std::cout << "  -v, --verbose       Enable verbose output (print AST)\n";
	std::cout << "  -h, --help          Show this help message\n";
}

} // namespace Phasor
