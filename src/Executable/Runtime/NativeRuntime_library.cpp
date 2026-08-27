#include "../../Runtime/Stdlib/StdLib.hpp"
#include "../../Runtime/Library/NativeRuntime.hpp"
#include "../../Frontend/Phasor/Frontend.hpp"
#include "../../Language/Phasor/Lexer/Lexer.hpp"
#include "../../Language/Phasor/Parser/Parser.hpp"
#include "../../Codegen/CodeGen.hpp"
#include "../../Codegen/IR/PhasorIR.hpp"
#include "../../Codegen/Bytecode/BytecodeSerializer.hpp"
#include "../../Codegen/Bytecode/BytecodeDeserializer.hpp"
#include "../../Language/Phasor/Parser/PlatformDefines.hpp"
#include <version.h>
#include <nativeerror.h>
#include <phsint.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

#include <cstring>
#include <vformat.hpp>

void set_terminal_title(const char *title)
{
#ifdef _WIN32
    SetConsoleTitleA(title);
#else
    vformat::printf("\033]0;%s\007", title);
    fflush(stdout);
#endif
}

#ifdef _WIN32
#define setupConsole() \
    AttachConsole(ATTACH_PARENT_PROCESS); \
    { FILE* _f; freopen_s(&_f, "CONOUT$", "w", stdout); } \
    { FILE* _f; freopen_s(&_f, "CONOUT$", "w", stderr); } \
    puts("")

Phasor::string getCommandLine(LPSTR &lpszCmdLine)
{
	Phasor::string cmdline = lpszCmdLine;
	return (cmdline.size() >= 2 && cmdline.starts_with('"') && cmdline.ends_with('"')) ? cmdline.substr(1, cmdline.size() - 2) : cmdline;
}
#endif

#ifndef _SHARED
#define PHASOR_API
#elif _WIN32
#define PHASOR_API __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define PHASOR_API __attribute__((visibility("default")))
#endif

#define msg error

static std::vector<std::filesystem::path> fetchIncludeDirs()
{
	std::vector<std::filesystem::path> finalPaths;

#ifdef PHASOR_DEFAULT_FIRST_PATH
	finalPaths.emplace_back(PHASOR_DEFAULT_FIRST_PATH);
#endif

	Phasor::string includeDirs;
	if (Phasor::dupenv_ret ret = Phasor::dupenv(includeDirs, "PHASOR_INCLUDE_PATH"); ret == Phasor::dupenv_ret::Success)
	{
		std::stringstream ss(includeDirs.c_str());
		std::string item;
		while (std::getline(ss, item, ';'))
		{
			if (!item.empty())
			{
				finalPaths.emplace_back(item);
			}
		}
	}

	finalPaths.push_back(std::filesystem::current_path());

	return finalPaths;
}

static std::vector<std::filesystem::path> resolveIncludePaths(const char *modulePath, const char **includePaths,
                                                            int includePathCount)
{
	std::vector<std::filesystem::path> finalPaths = fetchIncludeDirs();

	if (modulePath != nullptr && modulePath[0] != '\0')
	{
		finalPaths.emplace_back(modulePath);
	}

	if (includePaths == nullptr || includePathCount <= 0)
	{
		return finalPaths;
	}

	for (int i = 0; i < includePathCount; ++i)
	{
		if (includePaths[i] == nullptr || includePaths[i][0] == '\0')
		{
			continue;
		}

		Phasor::string raw(includePaths[i]);
		Phasor::string current;
		for (char ch : raw)
		{
			if (ch == ',')
			{
				if (!current.empty())
				{
					finalPaths.emplace_back(current.str());
					current.clear();
				}
			}
			else
			{
				current.push_back(ch);
			}
		}
		if (!current.empty())
		{
			finalPaths.emplace_back(current.str());
		}
	}

	return finalPaths;
}

static Phasor::Defines resolveDefines(const char **defines, int defineCount)
{
	Phasor::Defines finalDefines;
	Phasor::addDefaultDefines(finalDefines, false);

	if (defines == nullptr || defineCount <= 0)
	{
		return finalDefines;
	}

	for (int i = 0; i < defineCount; ++i)
	{
		if (defines[i] == nullptr || defines[i][0] == '\0')
		{
			continue;
		}

		Phasor::string raw(defines[i]);
		Phasor::string current;
		for (char ch : raw)
		{
			if (ch == ',')
			{
				if (!current.empty())
				{
					const Phasor::string item = current;
					const size_t eq = item.find('=');
					if (eq == Phasor::string::npos)
					{
						finalDefines[item] = Phasor::DefineValue(Phasor::DefineValueKind::Number, "1");
					}
					else
					{
						finalDefines[item.substr(0, eq)] = Phasor::parseCliDefineValue(item.substr(eq + 1));
					}
					current.clear();
				}
			}
			else
			{
				current.push_back(ch);
			}
		}
		if (!current.empty())
		{
			const Phasor::string item = current;
			const size_t eq = item.find('=');
			if (eq == Phasor::string::npos)
			{
				finalDefines[item] = Phasor::DefineValue(Phasor::DefineValueKind::Number, "1");
			}
			else
			{
				finalDefines[item.substr(0, eq)] = Phasor::parseCliDefineValue(item.substr(eq + 1));
			}
		}
	}

	return finalDefines;
}

extern "C"
{
	PHASOR_API const char *getVersion()
	{
		return PHASOR_VERSION_STRING;
	}

	PHASOR_API int exec(void *vmPtr, const unsigned char *bytecode, size_t bytecodeSize, const char *moduleName,
	                    int argc, const char **argv)
	{
		set_terminal_title(moduleName);
		try
		{
			std::vector<Phasor::u8>  bytecodeData(bytecode, bytecode + bytecodeSize);
			Phasor::NativeRuntime NativeRT(static_cast<Phasor::VM *>(vmPtr), bytecodeData, argc, argv);

			return NativeRT.run();
		}
		catch (const std::exception &e)
		{
			msg(Phasor::string(moduleName) + ": " + e.what());
		}
		return -1;
	}

	PHASOR_API int execFuncInt(void *vmPtr, const unsigned char *bytecode, size_t bytecodeSize, const char *moduleName,
	                           int argc, const char **argv, const char *functionName)
	{
		set_terminal_title(moduleName);
		try
		{
			std::vector<Phasor::u8>  bytecodeData(bytecode, bytecode + bytecodeSize);
			Phasor::NativeRuntime NativeRT(static_cast<Phasor::VM *>(vmPtr), bytecodeData, argc, argv);

			return NativeRT.runFunctionInt(functionName);
		}
		catch (const std::exception &e)
		{
			msg(Phasor::string(moduleName) + ": " + e.what());
		}
		return -1;
	}

	PHASOR_API const char *execFuncString(void *vmPtr, const unsigned char *bytecode, size_t bytecodeSize,
	                                      const char *moduleName, int argc, const char **argv, const char *functionName)
	{
		set_terminal_title(moduleName);
		static std::string ret;
		try
		{
			std::vector<Phasor::u8>  bytecodeData(bytecode, bytecode + bytecodeSize);
			Phasor::NativeRuntime NativeRT(static_cast<Phasor::VM *>(vmPtr), bytecodeData, argc, argv);

			auto result = NativeRT.runFunctionString(functionName);
			if (!result)
			{
				return nullptr;
			} 
				ret = *result;
			return ret.c_str();
		}
		catch (const std::exception &e)
		{
			msg(Phasor::string(moduleName) + ": " + e.what());
			return nullptr;
		}
	}

	PHASOR_API int evaluatePHS(void *vmPtr, const char *script, const char *moduleName, const char *modulePath,
	                           const char **includePaths, int includePathCount,
	                           const char **defines, int defineCount,
	                           bool verbose)
	{
		set_terminal_title(moduleName);
		try
		{
			Phasor::Lexer lexer(script);
			auto tokens = lexer.tokenize();
			Phasor::Parser parser(tokens, moduleName);
			auto resolvedIncludes = resolveIncludePaths(modulePath, includePaths, includePathCount);
			if (!resolvedIncludes.empty())
			{
				parser.setIncludePaths(resolvedIncludes);
			}
			if (modulePath != nullptr && modulePath[0] != '\0')
			{
				parser.setSourcePath(modulePath);
			}
			parser.setDefines(resolveDefines(defines, defineCount));
			auto program = parser.parse();
			if (verbose)
			{
				program->print();
			}
			Phasor::CodeGenerator codegen;
			auto bytecode = codegen.generate(*program);
			return static_cast<Phasor::VM *>(vmPtr)->run(bytecode);
		}
		catch (const std::exception &e)
		{
			msg(Phasor::string(moduleName) + ": " + e.what());
		}
		return -1;
	}

	PHASOR_API bool compilePHS(const char *script, const char *moduleName, const char *modulePath,
	                           const char **includePaths, int includePathCount,
	                           const char **defines, int defineCount,
	                           unsigned char *buffer, size_t bufferSize, size_t *outSize)
	{
		set_terminal_title((Phasor::string("Compiling ") + moduleName).c_str());
		try
		{
			Phasor::CodeGenerator      codegen;
			Phasor::BytecodeSerializer serializer;
			Phasor::Lexer              lexer(script);
			Phasor::Parser             parser(lexer.tokenize(), moduleName);
			auto includeDirs = resolveIncludePaths(modulePath, includePaths, includePathCount);
			if (!includeDirs.empty())
			{
				parser.setIncludePaths(includeDirs);
			}
			if (modulePath != nullptr && modulePath[0] != '\0')
			{
				parser.setSourcePath(modulePath);
			}
			parser.setDefines(resolveDefines(defines, defineCount));

			auto                 ast = parser.parse();
			auto                 bc = codegen.generate(*ast);
			std::vector<Phasor::u8> data = serializer.serialize(bc);

			if (outSize != nullptr)
			{
				*outSize = data.size();
			}

			if (buffer == nullptr)
			{
				return true;
			}

			if (bufferSize < data.size())
			{
				return false;
			}

			std::memcpy(buffer, data.data(), data.size());

			return true;
		}
		catch (const std::exception &e)
		{
			msg(Phasor::string(moduleName) + ": " + e.what());
		}
		return false;
	}

	PHASOR_API bool compilePHSToIR(const char *script, const char *moduleName, const char *modulePath,
	                           const char **includePaths, int includePathCount,
	                           const char **defines, int defineCount,
	                           unsigned char *buffer, size_t bufferSize, size_t *outSize)
	{
		set_terminal_title((Phasor::string("Compiling ") + moduleName).c_str());
		try
		{
			Phasor::CodeGenerator      codegen;
			Phasor::Lexer              lexer(script);
			Phasor::Parser             parser(lexer.tokenize(), moduleName);
			auto includeDirs = resolveIncludePaths(modulePath, includePaths, includePathCount);
			if (!includeDirs.empty())
			{
				parser.setIncludePaths(includeDirs);
			}
			if (modulePath != nullptr && modulePath[0] != '\0')
			{
				parser.setSourcePath(modulePath);
			}
			parser.setDefines(resolveDefines(defines, defineCount));

			auto                 ast = parser.parse();
			auto                 bc = codegen.generate(*ast);
			std::vector<Phasor::u8> data = Phasor::PhasorIR::serialize(bc);

			if (outSize != nullptr)
			{
				*outSize = data.size();
			}

			if (buffer == nullptr)
			{
				return true;
			}

			if (bufferSize < data.size())
			{
				return false;
			}

			std::memcpy(buffer, data.data(), data.size());

			return true;
		}
		catch (const std::exception &e)
		{
			msg(Phasor::string(moduleName) + ": " + e.what());
		}
		return false;
	}

	PHASOR_API bool assembleIR(const unsigned char *irBuffer, size_t irBufferSize,
	                          unsigned char *buffer, size_t bufferSize, size_t *outSize)
	{
		try
		{
			std::vector<Phasor::u8> irData(irBuffer, irBuffer + irBufferSize);
			auto bytecode = Phasor::PhasorIR::deserialize(irData);
			Phasor::BytecodeSerializer serializer;
			std::vector<Phasor::u8> data = serializer.serialize(bytecode);

			if (outSize != nullptr)
			{
				*outSize = data.size();
			}

			if (buffer == nullptr)
			{
				return true;
			}

			if (bufferSize < data.size())
			{
				return false;
			}

			std::memcpy(buffer, data.data(), data.size());
			return true;
		}
		catch (const std::exception &e)
		{
			msg(Phasor::string("assembleIR: ") + e.what());
		}
		return false;
	}

	PHASOR_API bool disassembleToIR(const unsigned char *bcBuffer, size_t bcBufferSize, unsigned char *buffer, size_t bufferSize, size_t *outSize)
	{
		try
		{
			Phasor::BytecodeDeserializer deserializer;
			std::vector<Phasor::u8> bcData(bcBuffer, bcBuffer + bcBufferSize);
			auto bytecode = deserializer.deserialize(bcData);
			std::vector<Phasor::u8> data = Phasor::PhasorIR::serialize(bytecode);

			if (outSize != nullptr)
			{
				*outSize = data.size();
			}

			if (buffer == nullptr)
			{
				return true;
			}

			if (bufferSize < data.size())
			{
				return false;
			}

			return true;

		}
		catch (const std::exception &e)
		{
			msg(Phasor::string("disassemble: ") + e.what());
		}
		return false;
	}

	PHASOR_API void *createState()
	{
		auto *vm = new Phasor::VM();
		return vm;
	}

	PHASOR_API void initStdLib(void *vmPtr)
	{
		Phasor::StdLib::registerFunctions(*static_cast<Phasor::VM *>(vmPtr));
	}

	PHASOR_API bool freeState(void *vmPtr)
	{
		if (vmPtr == nullptr) 
		{
			return false;
		}

		delete static_cast<Phasor::VM *>(vmPtr);
		return true;
	}

	PHASOR_API bool resetState(void *vmPtr, bool resetFunctions, bool resetVariables)
	{
		if (vmPtr == nullptr)
		{
			return false;
		}

		auto *vm = static_cast<Phasor::VM *>(vmPtr);
		vm->reset(true, resetFunctions, resetVariables);
		return true;
	}

	PHASOR_API bool isErrorStatus(void *vmPtr)
	{
		if (vmPtr == nullptr)
		{
			return false;
		}

		auto *vm = static_cast<Phasor::VM *>(vmPtr);
		return vm->isErrorStatus();
	}
}
