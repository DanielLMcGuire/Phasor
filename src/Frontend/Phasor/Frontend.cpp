#include "../../Language/Phasor/Lexer/Lexer.hpp"
#include "../../Language/Phasor/Parser/Parser.hpp"
#include "../../Codegen/IR/PhasorIR.hpp"
#include "../../Runtime/Stdlib/StdLib.hpp"
#include "../../Runtime/VM/VM.hpp"
#include "../../Runtime/FFI/ffi.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <version.h>
#include <sscanf.h>

#include "Frontend.hpp"
#include <nativeerror.h>

bool startsWith(const std::string &input, const std::string &prefix)
{
	if (input.size() >= prefix.size() && input.compare(0, prefix.size(), prefix) == 0)
	{
		return true;
	}
	return false;
}

int Phasor::Frontend::runScript(const std::string &source, VM *vm)
{
	int status = 0;
	bool ownVM = false;
	CodeGenerator codegen;
	Lexer lexer(source);
	Parser parser(lexer.tokenize());
	auto   program = parser.parse();
	auto          bytecode = codegen.generate(*program);

	if (vm == nullptr)
	{
		ownVM = true;
		vm = new VM();
		StdLib::registerFunctions(*vm);
	}

#if defined(_WIN32)
	FFI ffi("plugins", vm);
#elif defined(__APPLE__)
	FFI ffi("/Library/Application Support/org.Phasor.Phasor/plugins", vm);
#elif defined(__linux__)
	FFI ffi("/opt/Phasor/plugins", vm);
#endif

	vm->setImportHandler([](const std::filesystem::path &path) {
		std::ifstream file(path);
		if (!file.is_open())
		{
			throw std::runtime_error("Could not open imported file: " + path.string());
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		runScript(buffer.str());
	});

	if (status != 0) {
		if (ownVM) delete vm;
		return status;
	}

	status = vm->run(bytecode);

	if (ownVM)
	{
		delete vm;
	}

	return status;
}

int Phasor::Frontend::runRepl(VM *vm)
{
	int status = 0;
	bool ownVM = false;
	CodeGenerator              codegen;

	if (vm == nullptr)
	{
		ownVM = true;
		vm = new VM();
		StdLib::registerFunctions(*vm);
	}

#if defined(_WIN32)
	FFI ffi("plugins", vm);
#elif defined(__APPLE__)
	FFI ffi("/Library/Application Support/org.Phasor.Phasor/plugins", vm);
#elif defined(__linux__)
	FFI ffi("/opt/Phasor/plugins", vm);
#endif

	vm->setImportHandler([](const std::filesystem::path &path) {
		std::ifstream file(path);
		if (!file.is_open())
		{
			throw std::runtime_error("Could not open imported file: " + path.string());
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		runScript(buffer.str());
	});

	if (status != 0) {
		if (ownVM) delete vm;
		std::cout << "Failed to create FFI handler!";
		return status;
	}

	std::map<std::string, int> globalVars;
	int                        nextVarIdx = 0;	
	std::string line;
	bool cleanExit = false;

	std::cout << "Phasor REPL (using Phasor VM v" << PHASOR_VERSION_STRING << ")\n(C) 2026 Daniel McGuire\n\n";
	std::cout << "Type 'exit();' to quit. Function declarations will not work.\n";

	
	while (true)
	{
		try
		{
			std::cout << "\n> ";
			if (!std::getline(std::cin, line))
				break;
				
			if (startsWith(line, "exit"))
			{
				cleanExit = true;
				break;
			}
			if (line.empty())
			{
				std::cerr << "Empty line\n";
				continue;
			}

			Lexer lexer(line);
			Parser parser(lexer.tokenize());
			
			auto   program = parser.parse();
			auto bytecode = codegen.generate(*program, globalVars, nextVarIdx, true);

			globalVars = bytecode.variables;
			nextVarIdx = bytecode.nextVarIndex;

			status = vm->run(bytecode);
		}
		catch (const std::exception &e)
		{
			std::string errorMsg = std::string(e.what()) + " | " + vm->getInformation() + "\n";
			error(errorMsg);
		}
	}

	if (ownVM)
		delete vm;

	if (cleanExit)
		return 0;

	return status;
}
