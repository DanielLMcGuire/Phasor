#include "NativeRuntime.hpp"
#include "../../Backend/Lexer/Lexer.hpp"
#include "../../Backend/Parser/Parser.hpp"
#include "../../AST/AST.hpp"
#include "../../Codegen/CodeGen.hpp"
#include "../../Codegen/Bytecode/BytecodeDeserializer.hpp"
#include "../../Codegen/Bytecode/BytecodeSerializer.hpp"
#include "../../Codegen/Cpp/CppCodeGenerator.hpp"
#include "../../Runtime/VM/VM.hpp"
#include "../../Runtime/Stdlib/StdLib.hpp"
#include "../../Runtime/FFI/ffi.hpp"
#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#define error(msg) MessageBoxA(NULL, std::string(msg).c_str(), "Phasor Runtime Error", MB_OK | MB_ICONERROR)
#else
#define error(msg) std::cerr << "Error: " << msg << std::endl
#endif

namespace Phasor
{

NativeRuntime::NativeRuntime(const std::vector<uint8_t> &bytecodeData, const int argc, const char **argv)
    : m_bytecodeData(bytecodeData), m_argc(argc), m_argv(const_cast<char **>(argv))
{
	BytecodeDeserializer deserializer;
	m_bytecode = deserializer.deserialize(m_bytecodeData);
	m_vm = std::make_unique<VM>();
}

NativeRuntime::NativeRuntime(const std::string &script, const int argc, char **argv) : m_script(script), m_argc(argc), m_argv(argv)
{
	Lexer lexer(m_script);
	auto  tokens = lexer.tokenize();

	Parser parser(tokens);
	auto   program = parser.parse();

	CodeGenerator codegen;
	m_bytecode = codegen.generate(*program);
	m_vm = std::make_unique<VM>();
}

NativeRuntime::~NativeRuntime()
{
	m_vm.reset();
	m_vm = nullptr;
}

void NativeRuntime::addNativeFunction(const std::string &name, void *function)
{
	using RawFunctionPtr = Value (*)(const std::vector<Value> &, VM *);
	RawFunctionPtr rawPtr = reinterpret_cast<RawFunctionPtr>(function);
	NativeFunction nativeFunction = rawPtr;
	m_vm->registerNativeFunction(name, nativeFunction);
}

void NativeRuntime::eval(VM *vm, const std::string &script)
{
	Lexer lexer(script);
	auto  tokens = lexer.tokenize();

	Parser parser(tokens);
	auto   program = parser.parse();

	CodeGenerator codegen;
	auto          bytecode = codegen.generate(*program);

	vm->run(bytecode);
}

int NativeRuntime::run()
{

	try
	{
		StdLib::argc = m_argc;
		StdLib::argv = m_argv;
		StdLib::registerFunctions(*m_vm);
#if defined(_WIN32)
		FFI ffi("plugins", m_vm.get());
#elif defined(__APPLE__)
		FFI ffi("/Library/Application Support/org.Phasor.Phasor/plugins", m_vm.get());
#elif defined(__linux__)
		FFI ffi("/opt/Phasor/plugins", m_vm.get());
#endif
		m_vm->setImportHandler([](const std::filesystem::path &path) {
			throw std::runtime_error("Imports not supported in pure binary runtime yet: " + path.string());
		});

		m_vm->run(m_bytecode);
	}
	catch (const std::exception &e)
	{
		std::string errorMsg = std::string(e.what()) + " | " + m_vm->getInformation() + "\n";
		error(errorMsg);
		return 1;
	}
	return 0;
}

} // namespace Phasor
