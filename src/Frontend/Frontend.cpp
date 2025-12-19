#include "Frontend.hpp"
#include "../Backend/lexer/Lexer.hpp"
#include "../Backend/parser/Parser.hpp"
#include "../runtime/Stdlib/StdLib.hpp"
#include "../runtime/VM/VM.hpp"
#include "../Codegen/IR/PhasorIR.hpp" // Decode instructions
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

bool startsWith(const std::string &input, const std::string &prefix)
{
	if (input.size() >= prefix.size() && input.compare(0, prefix.size(), prefix) == 0)
	{
		return true;
	}
	return false;
}

bool endsWith(const std::string &input, const std::string &suffix)
{
	if (input.size() >= suffix.size() && input.compare(input.size() - suffix.size(), suffix.size(), suffix) == 0)
	{
		return true;
	}
	return false;
}

void Frontend::runScript(const std::string &source, VM *vm)
{
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

	vm->run(bytecode);

	if (ownVM)
	{
		delete vm;
	}
}

void Frontend::runRepl(VM *vm)
{
	std::cout << "Phasor REPL v0.1\n(C) 2025 Daniel McGuire\n\n";
	std::cout << "Type 'exit;' to quit.\n";

	bool ownVM = false;
	if (vm == nullptr)
	{
		ownVM = true;
		VM *vm = new VM();
		StdLib::registerFunctions(*vm);
	}

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

	std::map<std::string, int> globalVars;
	int                        nextVarIdx = 0;
	CodeGenerator              codegen;

	std::string line;
	while (true)
	{
		std::cout << "> ";
		if (!std::getline(std::cin, line))
			break;
		if (!endsWith(line, ";"))
		{
			std::cerr << "Missing semicolon at column " << line.length() + 1 << "\n";
			continue;
		}
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
#pragma warning(push)
#pragma warning(disable : 4477)
		if (startsWith(line, "vm_op"))
		{
			char         instruction[64];
			unsigned int operand;
			sscanf_s(line.c_str(), "vm_op %s %d", instruction, sizeof(instruction), &operand);
			vm->operation(PhasorIR::stringToOpCode(std::string(instruction)), operand);
			continue;
		}
		if (startsWith(line, "vm_getvar"))
		{
			int index;
			sscanf_s(line.c_str(), "vm_getvar %d", &index);
			line = "let getvarx = " + vm->getVariable(index).toString();
		}
		if (startsWith(line, "vm_setvar"))
		{
			char value[64];
			sscanf_s(line.c_str(), "vm_setvar %s", value, sizeof(value));
			line = "var setvarx = " + std::to_string(vm->addVariable(value));
		}
#pragma warning(pop)
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

		try
		{
			Lexer lexer(line);
			auto  tokens = lexer.tokenize();

			Parser parser(tokens);
			auto   program = parser.parse();

			auto bytecode = codegen.generate(*program, globalVars, nextVarIdx, true);

			// Update persistent state
			globalVars = bytecode.variables;
			nextVarIdx = bytecode.nextVarIndex;

			vm->run(bytecode);
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error: " << e.what() << "\n";
		}
	}
	if (ownVM)
	{
		delete vm;
	}
}