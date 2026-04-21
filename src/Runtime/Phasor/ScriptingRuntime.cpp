#include "ScriptingRuntime.hpp"
#include "../../Language/Phasor/Lexer/Lexer.hpp"
#include "../../Language/Phasor/Parser/Parser.hpp"
#include "../../Codegen/CodeGen.hpp"
#include "../../Frontend/Phasor/Frontend.hpp"
#include "../../Runtime/Stdlib/StdLib.hpp"
#include "../../Runtime/VM/VM.hpp"
#include <version.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <nativeerror.h>


namespace Phasor
{

ScriptingRuntime::ScriptingRuntime(int argc, char *argv[])
{
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

	auto vm = createVm();

	try
	{
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

int ScriptingRuntime::runSourceString(const std::string &source, VM &vm)
{
	Lexer  lexer(source);
	auto   tokens = lexer.tokenize();
	Parser parser(tokens, m_args.inputFile);
	auto   program = parser.parse();

	if (m_args.verbose)
	{
		std::println("AST:");
		program->print();
		std::println();
	}

	CodeGenerator codegen;
	auto          bytecode = codegen.generate(*program);

	return vm.run(bytecode);
}

std::unique_ptr<VM> ScriptingRuntime::createVm()
{
	auto vm = std::make_unique<VM>();
	StdLib::registerFunctions(*vm);
	StdLib::argv = m_args.scriptArgv;
	StdLib::argc = m_args.scriptArgc;

#if defined(_WIN32)
	vm->initFFI("plugins");
#elif defined(__APPLE__)
	vm->initFFI("/Library/Application Support/org.Phasor.Phasor/plugins");
#elif defined(__linux__)
	vm->initFFI("/usr/lib/phasor/plugins/");
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
		if (arg == "-c" || arg == "--command")
		{
			auto vm = createVm();
			runSourceString(argv[i + 1], *vm);
			int ret = vm->getStatus();
			exit(ret);
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
	std::println(
		"Phasor Scripting Runtime v{}\n"
		"(C) 2026 Daniel McGuire - Licensed under Apache 2.0\n\n"
		"Usage:\n"
		"  {} [options] [file.phs] [...script args]\n\n"
		"Options:\n"
		"  -v, --verbose       Enable verbose output (print AST)\n"
		"  -h, --help          Show this help message\n"
		"  -c, --command       Run a source string from argv",
		PHASOR_VERSION_STRING, filename
	);
}

} // namespace Phasor
