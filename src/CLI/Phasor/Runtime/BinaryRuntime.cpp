#include "BinaryRuntime.hpp"
#include "../../../Codegen/Bytecode/BytecodeDeserializer.hpp"
#include "../../../Runtime/Stdlib/StdLib.hpp"
#include "../../../Runtime/VM/VM.hpp"
#include <filesystem>
#include <iostream>
#include "../../../Runtime/FFI/ffi.hpp"

#ifdef _WIN32
#include <Windows.h>
#define error(msg) MessageBoxA(NULL, std::string(msg).c_str(), "Phasor Runtime Error", MB_OK | MB_ICONERROR)
#else
#define error(msg) std::cerr << "Error: " << msg << std::endl
#endif

namespace Phasor
{

BinaryRuntime::BinaryRuntime(int argc, char *argv[], char *envp[])
{
	m_args.envp = envp;
	parseArguments(argc, argv);
}

int BinaryRuntime::run()
{
	if (m_args.inputFile.empty())
	{
		std::cerr << "Error: No input file provided\n";
		return 1;
	}

	try
	{
		if (m_args.verbose)
			std::cerr << "DEBUG: Loading bytecode from: " << m_args.inputFile << std::endl;

		BytecodeDeserializer deserializer;
		Bytecode             bytecode = deserializer.loadFromFile(m_args.inputFile);

		if (m_args.verbose)
		{
			std::cerr << "DEBUG: Bytecode loaded successfully" << std::endl;
			std::cerr << "DEBUG: Instructions: " << bytecode.instructions.size() << std::endl;
			std::cerr << "DEBUG: Constants: " << bytecode.constants.size() << std::endl;
		}

		auto vm = std::make_unique<VM>();
		StdLib::registerFunctions(*vm);
		StdLib::argv = m_args.scriptArgv;
		StdLib::argc = m_args.scriptArgc;
		StdLib::envp = m_args.envp;

#if defined(_WIN32)
		FFI ffi("plugins", vm.get());
#elif defined(__APPLE__)
		FFI ffi("/Library/Application Support/org.Phasor.Phasor/plugins", vm.get());
#elif defined(__linux__)
		FFI ffi("/opt/Phasor/plugins", vm.get());
#endif

		vm->setImportHandler([](const std::filesystem::path &path) {
			throw std::runtime_error("Imports not supported in pure binary runtime yet: " + path.string());
		});

		if (m_args.verbose)
			std::cerr << "DEBUG: About to run bytecode" << std::endl;

		vm->run(bytecode);

		if (m_args.verbose)
			std::cerr << "DEBUG: Bytecode execution complete" << std::endl;

		return 0;
	}
	catch (const std::exception &e)
	{
		error(e.what());
		return 1;
	}
}

void BinaryRuntime::parseArguments(int argc, char *argv[])
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

void BinaryRuntime::showHelp(const std::string &programName)
{
	std::string filename = std::filesystem::path(programName).filename().string();
	std::cout << "Phasor Binary Runtime\n\n";
	std::cout << "Usage:\n";
	std::cout << "  " << filename << " [options] <file.phsb> [...script args]\n\n";
	std::cout << "Options:\n";
	std::cout << "  -v, --verbose       Enable verbose output\n";
	std::cout << "  -h, --help          Show this help message\n";
}

} // namespace Phasor
