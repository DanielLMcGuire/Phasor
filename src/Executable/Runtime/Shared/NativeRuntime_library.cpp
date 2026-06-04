#include "../../../Runtime/Stdlib/StdLib.hpp"
#include "../../../Runtime/Shared/NativeRuntime.hpp"
#include "../../../Frontend/Phasor/Frontend.hpp"
#include "../../../Language/Phasor/Lexer/Lexer.hpp"
#include "../../../Language/Phasor/Parser/Parser.hpp"
#include "../../../Codegen/CodeGen.hpp"
#include "../../../Codegen/Bytecode/BytecodeSerializer.hpp"
#include "../../../Codegen/Bytecode/BytecodeDeserializer.hpp"
#include <version.h>
#include <nativeerror.h>
#include <phsint.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

#include <cstring>
#include <vformat.hpp>

void set_terminal_title(const char *title) {
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

std::string getCommandLine(LPSTR &lpszCmdLine) {
	std::string cmdline = lpszCmdLine;
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

std::vector<std::filesystem::path> fetchIncludeDirs() {
	std::vector<std::filesystem::path> finalPaths;

#ifdef PHASOR_DEFAULT_FIRST_PATH
	finalPaths.push_back(PHASOR_DEFAULT_FIRST_PATH);
#endif

	Phasor::PhsString includeDirs;
	if (Phasor::dupenv_ret ret = Phasor::dupenv(includeDirs, "PHASOR_INCLUDE_PATH"); ret == Phasor::dupenv_ret::Success)
	{
		std::stringstream ss(includeDirs.c_str());
		std::string item;
		while (std::getline(ss, item, ';'))
		{
			if (!item.empty())
				finalPaths.push_back(item);
		}
	}

	finalPaths.push_back(std::filesystem::current_path());

	return finalPaths;
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
			msg(std::string(moduleName) + ": " + e.what());
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
			msg(std::string(moduleName) + ": " + e.what());
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
				return nullptr;
			else
				ret = *result;
			return ret.c_str();
		}
		catch (const std::exception &e)
		{
			msg(std::string(moduleName) + ": " + e.what());
			return nullptr;
		}
	}

	PHASOR_API int evaluatePHS(void *vmPtr, const char *script, const char *moduleName, const char *modulePath,
	                           bool verbose)
	{
		set_terminal_title(moduleName);
		try
		{
			auto includeDirs = fetchIncludeDirs();
			includeDirs.push_back(modulePath);
			return Phasor::Frontend::runScript(script, static_cast<Phasor::VM *>(vmPtr), includeDirs, verbose);
		}
		catch (const std::exception &e)
		{
			msg(std::string(moduleName) + ": " + e.what());
		}
		return -1;
	}

	PHASOR_API bool compilePHS(const char *script, const char *moduleName, const char *modulePath,
	                           unsigned char *buffer, size_t bufferSize, size_t *outSize)
	{
		set_terminal_title((std::string("Compiling ") + moduleName).c_str());
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
			std::vector<Phasor::u8> data = serializer.serialize(bc);

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
			msg(std::string(moduleName) + ": " + e.what());
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
		Phasor::StdLib::registerFunctions(*static_cast<Phasor::VM *>(vmPtr));
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

	PHASOR_API bool isErrorStatus(void *vmPtr)
	{
		if (!vmPtr)
			return false;

		Phasor::VM *vm = static_cast<Phasor::VM *>(vmPtr);
		return vm->isErrorStatus();
	}

#if defined(_SHARED) && defined(_WIN32)
	PHASOR_API void CALLBACK PhasorSourceStringEvaluateA(HWND hwnd, HINSTANCE, LPSTR lpszCmdLine, int)
	{
		setupConsole();
		int exitCode = evaluatePHS(NULL, getCommandLine(lpszCmdLine).c_str(), __func__, "", false);
		if (exitCode != 0)
		{
			std::string message = std::format("\nFailed with code {}\n", exitCode);
			MessageBoxA(hwnd, message.c_str(), __func__, MB_OK | MB_ICONERROR);
		}
	}

	PHASOR_API void CALLBACK PhasorSourceFileEvaluateA(HWND hwnd, HINSTANCE, LPSTR lpszCmdLine, int)
	{
		setupConsole();
		std::filesystem::path file = getCommandLine(lpszCmdLine);
		std::string scriptText;

		if (!std::filesystem::exists(file))
		{
			std::string message = std::format("File \"{}\" does not exist\n", file.filename().string());
			MessageBoxA(hwnd, message.c_str(), __func__, MB_OK | MB_ICONERROR);
			return;
		}
		
		std::ifstream fileStream(file);
		if (!fileStream)
		{
			std::string message = std::format("File \"{}\" could not be opened\n", file.filename().string());
			MessageBoxA(hwnd, message.c_str(), __func__, MB_OK | MB_ICONERROR);
			return;
		}

		fileStream.seekg(0, std::ios::end);
		scriptText.resize(static_cast<size_t>(fileStream.tellg()));
		fileStream.seekg(0, std::ios::beg);

		fileStream.read(scriptText.data(), scriptText.size());

		int exitCode = evaluatePHS(NULL, scriptText.c_str(), __func__, file.parent_path().string().c_str(), false);
		if (exitCode != 0)
		{
			std::string message = std::format("\nFailed with code {}\n", exitCode);
			MessageBoxA(hwnd, message.c_str(), __func__, MB_OK | MB_ICONERROR);
		}
	}

	PHASOR_API void CALLBACK PhasorBytecodeFileExecuteA(HWND hwnd, HINSTANCE, LPSTR lpszCmdLine, int)
	{
		setupConsole();
		std::filesystem::path file = getCommandLine(lpszCmdLine);
		if (!std::filesystem::exists(file))
		{
			std::string message = std::format("File \"{}\" does not exist\n", file.filename().string());
			MessageBoxA(hwnd, message.c_str(), __func__, MB_OK | MB_ICONERROR);
			return;
		}
		if (file.extension().string() != ".phsb") 
		{
			std::string message = std::format("File \"{}\" is not a .phsb file\n", file.filename().string());
			MessageBoxA(hwnd, message.c_str(), __func__, MB_OK | MB_ICONERROR);
			return;
		}
		Phasor::BytecodeDeserializer deserializer;
		auto bytecode = deserializer.loadFromFile(file);
		std::array<const char *, 2> args = {"phasorrt.dll", __func__};
		Phasor::NativeRuntime NativeRT(bytecode, static_cast<int>(args.size()), args.data());
		int exitCode = NativeRT.run();
		if (exitCode != 0)
		{
			std::string message = std::format("\nFailed with code {}\n", exitCode);
			MessageBoxA(hwnd, message.c_str(), __func__, MB_OK | MB_ICONERROR);
		}
	}
#endif // if _SHARED && _WIN32
}
