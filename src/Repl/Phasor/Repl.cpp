#include "Repl.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include "../../Codegen/Bytecode/BytecodeDeserializer.hpp"
#include "../../Codegen/Bytecode/BytecodeSerializer.hpp"
#include "../../Codegen/CodeGen.hpp"
#include "../../Frontend/Phasor/Frontend.hpp"
#include "../../Language/Phasor/Lexer/Lexer.hpp"
#include "../../Language/Phasor/Parser/Parser.hpp"
#include "../../Runtime/Stdlib/StdLib.hpp"
#include "../../Runtime/VM/VM.hpp"
#include "../../Codegen/IR/PhasorIR.hpp"

namespace Phasor
{

Repl::Repl(int argc, char *argv[], char *envp[])
{
	m_args.scriptArgc = argc - 1;
	m_args.scriptArgv = argv + 1;
	m_args.envp = envp;
}

int Repl::run()
{
	return runRepl();
}

int Repl::runRepl()
{
	auto vm = createVm();
	return Frontend::runRepl(vm.get());
}

int Repl::runSourceString(const std::string &source, VM &vm)
{
	Lexer  lexer(source);
	auto   tokens = lexer.tokenize();
	Parser parser(tokens);
	auto   program = parser.parse();

	CodeGenerator codegen;
	auto          bytecode = codegen.generate(*program);

	return vm.run(bytecode);
}

std::unique_ptr<VM> Repl::createVm()
{
	auto vm = std::make_unique<VM>();
	StdLib::registerFunctions(*vm);
	StdLib::argv = m_args.scriptArgv;
	StdLib::argc = m_args.scriptArgc;
	StdLib::envp = m_args.envp;

	vm->setImportHandler([vm_ptr = vm.get()](const std::filesystem::path &path) {
		std::ifstream file(path);
		if (!file.is_open())
			throw std::runtime_error("Could not open imported file: " + path.string());
		std::stringstream buffer;
		buffer << file.rdbuf();
		runSourceString(buffer.str(), *vm_ptr);
	});

	return vm;
}

} // namespace Phasor