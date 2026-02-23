#include "../../Language/Pulsar/Lexer/Lexer.hpp"
#include "../../Language/Pulsar/Parser/Parser.hpp"
#include "../../Codegen/IR/PhasorIR.hpp"
#include "../../Runtime/Stdlib/StdLib.hpp"
#include "../../Runtime/VM/VM.hpp"
#include "../../Runtime/FFI/ffi.hpp"

#include <version.h>
#include <sscanf.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "Frontend.hpp"

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

int pulsar::Frontend::runScript(const std::string &source, Phasor::VM *vm)
{
	Lexer                 lexer(source);
	Parser                parser(lexer.tokenize());
	Phasor::CodeGenerator codegen;
	auto                  program = parser.parse();
	auto                  bytecode = codegen.generate(*program);

	int status = 0;
	bool ownVM = false;

	if (vm == nullptr)
	{
		ownVM = true;
		vm = new Phasor::VM();
		Phasor::StdLib::registerFunctions(*vm);
	}

#if defined(_WIN32)
	Phasor::FFI ffi("plugins", vm);
#elif defined(__APPLE__)
	Phasor::FFI ffi("/Library/Application Support/org.Phasor.Phasor/plugins", vm);
#elif defined(__linux__)
	Phasor::FFI("/opt/Phasor/plugins", vm);
#endif

	if (status != 0) {
		if (ownVM) delete vm;
		return status;
	}

	InstanceHandle handle = vm->load(bytecode);
	status = vm->execute(handle);

	if (ownVM)
	{
		delete vm;
	}
	return status;
}

int pulsar::Frontend::runRepl(Phasor::VM *vm)
{
	std::cout << "Pulsar REPL (using Phasor VM v" << PHASOR_VERSION_STRING << ")\n(C) 2026 Daniel McGuire\n\n";
	std::cout << "Type 'exit()' to quit. Function declarations will not work.\n";

	int status = 0;
	bool ownVM = false;
	if (vm == nullptr)
	{
		ownVM = true;
		vm = new Phasor::VM();
		Phasor::StdLib::registerFunctions(*vm);
	}

#if defined(_WIN32)
	Phasor::FFI ffi("plugins", vm);
#elif defined(__APPLE__)
	Phasor::FFI ffi("/Library/Application Support/org.Phasor.Phasor/plugins", vm);
#elif defined(__linux__)
	Phasor::FFI ffi("/opt/Phasor/plugins", vm);
#endif

	if (status != 0) {
		if (ownVM) delete vm;
		return status;
	}

	std::map<std::string, int> globalVars;
	int                        nextVarIdx = 0;
	Phasor::CodeGenerator      codegen;

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
				vm->operation(Phasor::PhasorIR::stringToOpCode(std::string(instruction)), operand);
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
				// TODO Update this
				continue;
			}
			if (startsWith(line, "exit"))
				break;
			if (line.empty())
			{
				std::cerr << "Empty line\n";
				continue;
			}

			Lexer  lexer(line);
			Parser parser(lexer.tokenize());
			auto   program = parser.parse();
			auto   bytecode = codegen.generate(*program, globalVars, nextVarIdx, true);

			globalVars = bytecode.variables;
			nextVarIdx = bytecode.nextVarIndex;

			InstanceHandle handle = vm->load(bytecode);
			vm->execute(handle);
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
	return 0;
}
