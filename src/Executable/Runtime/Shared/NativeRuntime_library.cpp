#include "../../../Runtime/Stdlib/StdLib.hpp"
#include "../../../Runtime/Shared/NativeRuntime.hpp"
#include "../../../Frontend/Pulsar/Frontend.hpp"
#include "../../../Frontend/Phasor/Frontend.hpp"
#include "../../../Language/Phasor/Lexer/Lexer.hpp"
#include "../../../Language/Phasor/Parser/Parser.hpp"
#include "../../../Language/Pulsar/Lexer/Lexer.hpp"
#include "../../../Language/Pulsar/Parser/Parser.hpp"
#include "../../../Codegen/CodeGen.hpp"
#include "../../../Codegen/Bytecode/BytecodeSerializer.hpp"

#ifdef _WIN32
#define PHASOR_API __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define PHASOR_API __attribute__((visibility("default")))
#endif

#include <vector>
#include <cstring>
#include <sstream>

extern "C"
{
#ifdef _WIN32
	PHASOR_API int exec(void *vmPtr, const unsigned char *bytecode, size_t bytecodeSize, const char *moduleName,
	                    int argc, const char **argv)
#else
	PHASOR_API int exec(void *vmPtr, const unsigned char *bytecode, size_t bytecodeSize, const char *,
	                    int argc, const char **argv)
#endif
	{
		try
		{
			std::vector<uint8_t>  bytecodeData(bytecode, bytecode + bytecodeSize);
			Phasor::NativeRuntime NativeRT(static_cast<Phasor::VM *>(vmPtr), bytecodeData, argc, argv);

			return NativeRT.run();
		}
		catch (const std::exception &e)
		{
#ifdef _WIN32
			MessageBoxA(nullptr, e.what(), (std::string(moduleName) + " | Phasor Runtime - Error").c_str(),
			            MB_OK | MB_ICONERROR);
#else
			std::cerr << "Error: " << e.what() << "\n\a";
#endif
		}
		return -1;
	}
#ifdef _WIN32
	PHASOR_API int evaluatePHS(void *vmPtr, const char *script, const char *moduleName, 
								const char *modulePath, bool verbose)
#else
	PHASOR_API int evaluatePHS(void *vmPtr, const char *script, const char *,
								const char *modulePath, bool verbose)
#endif
	{
		try
		{
			return Phasor::Frontend::runScript(script, static_cast<Phasor::VM *>(vmPtr), modulePath, verbose);
		}
		catch (const std::exception &e)
		{
#ifdef _WIN32
			MessageBoxA(nullptr, e.what(), (std::string(moduleName) + " | Phasor Runtime - Error").c_str(),
			            MB_OK | MB_ICONERROR);
#else
			std::cerr << "Error: " << e.what() << "\n\a";
#endif
		}
		return -1;
	}

#ifdef _WIN32
	PHASOR_API int evaluatePUL(void *vmPtr, const char *script, const char *moduleName)
#else
	PHASOR_API int evaluatePUL(void *vmPtr, const char *script, const char *)
#endif
	{
		try
		{
			return pulsar::Frontend::runScript(script, static_cast<Phasor::VM *>(vmPtr));
		}
		catch (const std::exception &e)
		{
#ifdef _WIN32
			MessageBoxA(nullptr, e.what(), (std::string(moduleName) + " | Phasor Runtime - Error").c_str(),
			            MB_OK | MB_ICONERROR);
#else
			std::cerr << "Error: " << e.what() << "\n\a";
#endif
		}
		return -1;
	}

	PHASOR_API bool compilePHS(const char *script, const char *moduleName, const char *modulePath,
	                           unsigned char *buffer, size_t bufferSize, size_t *outSize)
	{
		try
		{
			Phasor::CodeGenerator      codegen;
			Phasor::BytecodeSerializer serializer;
			Phasor::Lexer              lexer(script);
			Phasor::Parser             parser(lexer.tokenize());

			if (modulePath && std::filesystem::exists(modulePath))
			{
				parser.setSourcePath(modulePath);
			} 

			auto                 ast = parser.parse();
			auto                 bc = codegen.generate(*ast);
			std::vector<uint8_t> data = serializer.serialize(bc);

			if (outSize)
				*outSize = data.size();

			if (!buffer)
				return true;

			if (bufferSize < data.size())
				return false;

			std::memcpy(buffer, data.data(), data.size());

			return true;
		}
		catch (const std::exception &e)
		{
#ifdef _WIN32
			MessageBoxA(nullptr, e.what(), (std::string(moduleName) + " | Phasor Runtime - Error").c_str(),
			            MB_OK | MB_ICONERROR);
#else
			std::cerr << "Error: " << e.what() << "\n\a";
#endif
		}
		return false;
	}

	PHASOR_API bool compilePUL(const char *script, const char *moduleName, unsigned char *buffer, size_t bufferSize,
	                           size_t *outSize)
	{
		try
		{
			Phasor::CodeGenerator      codegen;
			Phasor::BytecodeSerializer serializer;
			pulsar::Lexer              lexer(script);
			pulsar::Parser             parser(lexer.tokenize());

			auto                 ast = parser.parse();
			auto                 bc = codegen.generate(*ast);
			std::vector<uint8_t> data = serializer.serialize(bc);

			if (outSize)
				*outSize = data.size();

			if (!buffer)
				return true;

			if (bufferSize < data.size())
				return false;

			std::memcpy(buffer, data.data(), data.size());

			return true;
		}
		catch (const std::exception &e)
		{
#ifdef _WIN32
			MessageBoxA(nullptr, e.what(), (std::string(moduleName) + " | Phasor Runtime - Error").c_str(),
			            MB_OK | MB_ICONERROR);
#else
			std::cerr << "Error: " << e.what() << "\n\a";
#endif
		}
		return false;
	}

	PHASOR_API void *createState()
	{
		auto vm = new Phasor::VM();
		return vm;
	}

	PHASOR_API void initStdLib(void *vmPtr)
	{
		auto vm = static_cast<Phasor::VM *>(vmPtr);
		Phasor::StdLib::registerFunctions(*vm);
	}

	PHASOR_API bool freeState(void *vmPtr)
	{
		if (!vmPtr)
			return false;

		delete static_cast<Phasor::VM *>(vmPtr);
		return true;
	}

	PHASOR_API bool resetState(void *vmPtr, bool resetFunctions, bool resetVariables)
	{
		if (!vmPtr)
			return false;

		Phasor::VM *vm = static_cast<Phasor::VM *>(vmPtr);
		vm->reset(true, resetFunctions, resetVariables);
		return true;
	}
}