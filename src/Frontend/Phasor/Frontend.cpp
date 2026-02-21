#include "../../Language/Phasor/Lexer/Lexer.hpp"
#include "../../Language/Phasor/Parser/Parser.hpp"
#include "../../Codegen/IR/PhasorIR.hpp"
#include "../../Runtime/Stdlib/StdLib.hpp"
#include "../../Runtime/VM/VM.hpp"
#include "Frontend.hpp"

#include <version.h>

#include "../../Runtime/FFI/ffi.hpp"
#include <sscanf.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#define error(msg) MessageBoxA(NULL, std::string(msg).c_str(), "Phasor Runtime Error", MB_OK | MB_ICONERROR)
#else
#define error(msg) std::cerr << "Error: " << msg << std::endl
#endif

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
	Lexer lexer(source);
	auto  tokens = lexer.tokenize();

	Parser parser(tokens);
	auto   program = parser.parse();

	CodeGenerator codegen;
	auto          bytecode = codegen.generate(*program);

	bool ownVM = false;

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

	vm->setImportHandler([&status](const std::filesystem::path &path) {
		std::ifstream file(path);
		if (!file.is_open())
		{
			throw std::runtime_error("Could not open imported file: " + path.string());
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		status = runScript(buffer.str());
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
	std::cout << "Phasor REPL (using Phasor VM v" << PHASOR_VERSION_STRING << ")\n(C) 2026 Daniel McGuire\n\n";
	std::cout << "Type 'exit();' to quit. Function declarations will not work.\n";
	int status = 0;
	bool ownVM = false;
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

	vm->setImportHandler([&status](const std::filesystem::path &path) {
		std::ifstream file(path);
		if (!file.is_open())
		{
			throw std::runtime_error("Could not open imported file: " + path.string());
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		status = runScript(buffer.str());
	});

	if (status != 0) {
		if (ownVM) delete vm;
		return status;
	}

	std::map<std::string, int> globalVars;
	int                        nextVarIdx = 0;
	CodeGenerator              codegen;

	std::string line;
	while (true)
	{
		try
		{
			std::cout << "\n> ";
			if (!std::getline(std::cin, line))
				break;
			if (startsWith(line, "vm_pop"))
			{
				line = "let popx = " + vm->pop().toString();
				continue;
			}
			if (startsWith(line, "vm_push"))
			{
				vm->push(line.substr(4));
				continue;
			}
			if (startsWith(line, "vm_peek"))
			{
				line = "let peekx = " + vm->peek().toString();
			}
			if (startsWith(line, "vm_op"))
			{
				char         instruction[64];
				unsigned int operand;
				sscanf(line.c_str(), "vm_op %63s %d", instruction, &operand);
				vm->operation(PhasorIR::stringToOpCode(std::string(instruction)), operand);
				continue;
			}
			if (startsWith(line, "vm_getvar"))
			{
				int index;
				sscanf(line.c_str(), "vm_getvar %d", &index);
				line = "let getvarx = " + vm->getVariable(index).toString();
			}
			if (startsWith(line, "vm_setvar"))
			{
				char value[64];
				sscanf(line.c_str(), "vm_setvar %63s", value);
				line = "var setvarx = " + std::to_string(vm->addVariable(value));
			}
			if (startsWith(line, "vm_reset"))
			{
				vm->reset(true, true, true);
				continue;
			}
			if (startsWith(line, "exit"))
				break;
			if (line.empty())
			{
				std::cerr << "Empty line\n";
				continue;
			}

			Lexer lexer(line);
			auto  tokens = lexer.tokenize();

			Parser parser(tokens);
			auto   program = parser.parse();

			auto bytecode = codegen.generate(*program, globalVars, nextVarIdx, true);

			// Update persistent state
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
	{
		delete vm;
	}

	return status;
}
