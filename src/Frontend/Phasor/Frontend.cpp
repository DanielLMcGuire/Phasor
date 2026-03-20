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

int Phasor::Frontend::runScript(const std::string &source, VM *vm, const std::filesystem::path &path, bool verbose)
{
	int status = 0;
	bool ownVM = false;
	CodeGenerator codegen;
	Lexer lexer(source);
	Parser parser(lexer.tokenize());
	if (!path.empty() && std::filesystem::exists(path)) {
		parser.setSourcePath(path);
	} 
	
	auto   program = parser.parse();
#ifndef TRACING
	if (verbose)
	{
#endif
		std::cout << "AST:\n";
		program->print();
		std::cout << "\n";
#ifndef TRACING
	}
#endif
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

	vm->setImportHandler([vm](const std::filesystem::path &path) {
		std::ifstream file(path);
		if (!file.is_open())
		{
			throw std::runtime_error("Could not open imported file: " + path.string());
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		runScript(buffer.str(), vm, path);
	});

	if (status != 0) {
		if (ownVM) delete vm;
		return status;
	}

	try
	{
		status = vm->run(bytecode);
	}
	catch (...)
	{
		if (ownVM) delete vm;
		throw;
	}

	if (ownVM) delete vm;

	return status;
}

int Phasor::Frontend::runRepl(VM *vm, bool verbose)
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

	vm->setImportHandler([vm](const std::filesystem::path &path) {
		std::ifstream file(path);
		if (!file.is_open())
		{
			throw std::runtime_error("Could not open imported file: " + path.string());
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		runScript(buffer.str(), vm, path);
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
				
			if (line.starts_with("exit"))
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
#ifndef TRACING
			if (verbose)
			{
#endif
				std::cout << "AST:\n";
				program->print();
				std::cout << "\n";
#ifndef TRACING
			}
#endif
			auto bytecode = codegen.generate(*program, globalVars, nextVarIdx, true);

			globalVars = bytecode.variables;
			nextVarIdx = bytecode.nextVarIndex;

			status = vm->run(bytecode);
		}
		catch (const std::exception &e)
		{
			error(std::format("{}\n", e.what()));
		}
	}

	if (ownVM)
		delete vm;

	if (cleanExit)
		return 0;

	return status;
}
