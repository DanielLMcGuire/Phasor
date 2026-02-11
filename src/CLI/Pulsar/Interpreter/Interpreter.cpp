#include "Interpreter.hpp"
#include "../../../Language/Pulsar/Lexer/Lexer.hpp"
#include "../../../Language/Pulsar/Parser/Parser.hpp"
#include "../../../Codegen/CodeGen.hpp"
#include "../../../Frontend/Pulsar/Frontend.hpp"
#include "../../../Runtime/Stdlib/StdLib.hpp"
#include "../../../Runtime/VM/VM.hpp"
#include "../../../Runtime/FFI/ffi.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#define error(msg) MessageBoxA(NULL, std::string(msg).c_str(), "Phasor VM Runtime Error", MB_OK | MB_ICONERROR)
#else
#define error(msg) std::cerr << "Error: " << msg << std::endl
#endif

namespace pulsar
{

Interpreter::Interpreter(int argc, char *argv[], char *envp[])
{
	m_args.envp = envp;
	parseArguments(argc, argv);
}

int Interpreter::run()
{
	if (m_args.inputFile.empty())
		return 0;

	return runSource();
}

int Interpreter::runSource()
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

void Interpreter::runSourceString(const std::string &source, Phasor::VM &vm)
{
	Lexer  lexer(source);
	Parser        parser(lexer.tokenize());
	Phasor::CodeGenerator codegen;
	auto   program = parser.parse();

	if (m_args.verbose)
	{
		std::cout << "AST:\n";
		program->print();
		std::cout << "\n";
	}
	auto          bytecode = codegen.generate(*program);

	vm.run(bytecode);
}

std::unique_ptr<Phasor::VM> Interpreter::createVm()
{
	auto vm = std::make_unique<Phasor::VM>();
	Phasor::StdLib::registerFunctions(*vm);
	Phasor::StdLib::argv = m_args.scriptArgv;
	Phasor::StdLib::argc = m_args.scriptArgc;
	Phasor::StdLib::envp = m_args.envp;

#if defined(_WIN32)
	Phasor::FFI ffi("plugins", vm.get());
#elif defined(__APPLE__)
	Phasor::FFI ffi("/Library/Application Support/org.Phasor.Phasor/plugins", vm.get());
#elif defined(__linux__)
	Phasor::FFI ffi("/opt/Phasor/plugins", vm.get());
#endif

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

void Interpreter::parseArguments(int argc, char *argv[])
{
	m_args.program = std::filesystem::path(argv[0]);
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
			showHelp();
			exit(0);
		}
		else
		{
			defaultArgLocation = i;
			m_args.inputFile = arg;
			break;
		}
	}
	m_args.scriptArgv = argv + defaultArgLocation;
	m_args.scriptArgc = argc - defaultArgLocation;
}

void Interpreter::showHelp()
{
	std::cout << "Usage:\n" << "  " << m_args.program.stem().string() << " [inFile] [...script args]\n\n";
}

} // namespace Phasor
