// Copyright 2025-2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://phasor.pages.dev/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "../../Runtime/VM/VM.hpp"
#include "../../Runtime/Stdlib/StdLib.hpp"
#include "../../Language/Phasor/Lexer/Lexer.hpp"
#include "../../Language/Phasor/Parser/Parser.hpp"
#include "../../Language/Phasor/Parser/PlatformDefines.hpp"
#include "../../Codegen/CodeGen.hpp"
#include "../../Codegen/Bytecode/BytecodeDeserializer.hpp"

#include <print>
#include <format>
#include <PhasorString.hpp>
#include <vector>
#include <filesystem>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iterator>
#include <unordered_map>

#include <phs_dupenv.hpp>
#include <nativeerror.h>
#include <version.h>

#ifdef _WIN32
#include <io.h>
#define IS_TERMINAL _isatty(_fileno(stdin))
#else
#include <unistd.h>
#define IS_TERMINAL isatty(fileno(stdin))
#endif

/**
 * @brief Reads all content from stdin until EOF (piped input)
 */
Phasor::string readStdin()
{
	Phasor::string content;
	std::string line;
	while (std::getline(std::cin, line))
	{
		content += line + "\n";
	}
	return content;
}

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

#ifndef PHS_WINDOWED
void showHelp(const std::filesystem::path &program = "phasor")
#else
void showHelp(const std::filesystem::path &program = "phasorw")
#endif
{
	const Phasor::string programName = program.stem().string();
	std::println("Phasor Programming Language and Toolchain v{}\n"
	             "(C) 2026 Daniel McGuire - Licensed under Apache 2.0\n\n"
				 "See more with phasor-help phasor\n"
	             "Usage: [SCRIPT] | {} [OPTIONS...] [FILE] [--] [SCRIPTARGS...]\n"
	             "A. PIPE:    <text> | {} [--] [SCRIPTARGS...]\n"
	             "B. COMMAND: {} -c \"script\" [--] [SCRIPTARGS...]\n"
	             "C. FILE:    {} <file> [--] [SCRIPTARGS...]\n"
	             "D. REPL:    {}\n\n"
	             "Example:",
	             PHASOR_VERSION_STRING, programName, programName, programName, programName, programName);

#ifdef _WIN32
	std::println("A. CMD:  echo \"print(^\"Hi\\!\\n^\");\" | {}\n"
	             "A. PWSH: \"print(`\"Hi\\!\\n`\");\" | {}\n"
	             "B.       {} hello.phs\n"
	             "B.       {} hello.phsb\n"
                 "B.       {} hello.phs -- -myScriptFlag",
	             programName, programName, programName, programName, programName);
#else
	std::println("A. echo \"print(\\\"Hi\\!\\\\\\\\n\\\");\" | {}\n"
	             "B. {} hello.phs\n"
	             "B. {} hello.phsb\n"
                 "B. {} hello.phs -- -myScriptFlag",
	             programName, programName, programName, programName);
#endif
	std::println(R"(
Options:
    -h, --help     Show this help message and exit
    -v, --version  Show the version number and exit
    -c, --command  Run a raw script string
    -D, --define   Add a NAME[=VALUE] definition (comma-separated, repeatable)
    --verbose      Print the parsed AST before running)");
}

std::unique_ptr<Phasor::VM> createVm(int scriptArgc, char **scriptArgv)
{
	auto vm = std::make_unique<Phasor::VM>();
	Phasor::StdLib::registerFunctions(*vm);
	Phasor::StdLib::argc = scriptArgc;
	Phasor::StdLib::argv = scriptArgv;

#if defined(_WIN32)
	vm->initFFI({"phasornative", "plugins"});
#elif defined(__APPLE__)
	vm->initFFI({"phasornative", "/Library/Application Support/org.Phasor.Phasor/plugins"});
#else
	vm->initFFI({"phasornative", "/usr/lib/phasor/plugins/"});
#endif

	return vm;
}

int runSourceString(const Phasor::string &source, Phasor::VM &vm, const std::vector<std::filesystem::path> &includePaths,
                     const Phasor::string &sourceName, bool verbose,
                     const Phasor::Defines &defines)
{
	Phasor::Lexer  lexer(source);
	auto           tokens = lexer.tokenize();
	Phasor::Parser parser(tokens, sourceName.str());
	if (!includePaths.empty())
	{
		parser.setIncludePaths(includePaths);
	}
	if (!defines.empty())
	{
		parser.setDefines(defines);
	}
	auto program = parser.parse();

	if (verbose)
	{
		std::println("AST:");
		program->print();
		std::println();
	}

	Phasor::CodeGenerator codegen;
	auto                  bytecode = codegen.generate(*program);

	return vm.run(bytecode);
}

int runScriptFile(const std::filesystem::path &file, int scriptArgc, char **scriptArgv, const std::vector<std::filesystem::path> &includePaths,
                   bool verbose, const Phasor::Defines &defines)
{
	std::ifstream fileStream(file);
	if (!fileStream.is_open())
	{
		std::println(std::cerr, "Could not open file: {}", file.string());
		return 1;
	}

	std::stringstream buffer;
	buffer << fileStream.rdbuf();
	const Phasor::string source = buffer.str();

	auto vm = createVm(scriptArgc, scriptArgv);

	try
	{
		return runSourceString(source, *vm, includePaths, file.string(), verbose, defines);
	}
	catch (const std::exception &e)
	{
		Phasor::string errorMsg = Phasor::string(e.what()) + "\n";
		error(errorMsg);
		return 1;
	}
}

int runBytecodeFile(const std::filesystem::path &file, int scriptArgc, char **scriptArgv, bool verbose)
{
	try
	{
		if (verbose) 
		{
			std::println(std::cerr, "DEBUG: Loading bytecode from: {}", file.string());
		}

		Phasor::BytecodeDeserializer deserializer;
		Phasor::Bytecode            bytecode = deserializer.loadFromFile(file.string());

		if (verbose)
		{
			std::println(std::cerr, "DEBUG: Bytecode loaded successfully");
			std::println(std::cerr, "DEBUG: Instructions: {}", bytecode.instructions.size());
			std::println(std::cerr, "DEBUG: Constants: {}", bytecode.constants.size());
		}

		auto vm = createVm(scriptArgc, scriptArgv);

		if (verbose) 
		{
			std::println(std::cerr, "DEBUG: About to run bytecode");
		}

		int status = vm->run(bytecode);

		if (verbose)
		{
			std::println(std::cerr, "DEBUG: Bytecode execution complete with return {}", status);
		}

		return status;
	}
	catch (const std::exception &e)
	{
		error(e.what());
		return 1;
	}
}

int runRepl(const std::vector<std::filesystem::path> &includePaths, bool verbose,
            const Phasor::Defines &defines)
{
	auto vm = createVm(0, nullptr);

	std::unordered_map<std::string, int> globalVars;
	int                                  nextVarIdx = 0;
	std::string                          line;
	int                                  status = 0;
	bool                                 cleanExit = false;

	std::println("Phasor REPL (using Phasor VM v{})\n"
	             "(C) 2026 Daniel McGuire - Licensed under Apache 2.0\n\n"
	             "Type 'exit();' to quit. Function declarations will not work.",
	             PHASOR_VERSION_STRING);

	while (true)
	{
		try
		{
			std::print("\n> ");
			if (!std::getline(std::cin, line))
			{
				break;
			}

			if (line.starts_with("exit"))
			{
				cleanExit = true;
				break;
			}
			if (line.empty())
			{
				std::println(std::cerr, "Empty line");
				continue;
			}

			Phasor::Lexer  lexer(line);
			Phasor::Parser parser(lexer.tokenize());
			parser.setIncludePaths(includePaths);
			if (!defines.empty())
			{
				parser.setDefines(defines);
			}

			auto program = parser.parse();

			if (verbose)
			{
				std::println("AST:");
				program->print();
				std::println();
			}

			Phasor::CodeGenerator codegen;
			auto                  bytecode = codegen.generate(*program, globalVars, nextVarIdx, true);

			globalVars = bytecode.variables;
			nextVarIdx = bytecode.nextVarIndex;

			status = vm->run(bytecode);
		}
		catch (const std::exception &e)
		{
			error(std::format("{}\n", e.what()));
		}
	}

	return cleanExit ? 0 : status;
}

#ifndef _SHARED
#define PHASOR_API
#elif _WIN32
#define PHASOR_API __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define PHASOR_API __attribute__((visibility("default")))
#endif

extern "C"
{
PHASOR_API int phasor_main(int argc, char *argv[])
{
	try
	{
		const std::filesystem::path              programPath = argv[0];
		const std::vector<std::filesystem::path> includePaths = fetchIncludeDirs();

		bool                     verbose = false;
		bool                     is_parsing_options = true;
		bool                     has_command = false;
		Phasor::string              command_script;
		Phasor::string              file_script;
		std::vector<Phasor::string> script_args_storage;
		std::vector<Phasor::string>       defines_raw;

		for (int i = 1; i < argc; ++i)
		{
			Phasor::string arg = argv[i];
			if (!is_parsing_options)
			{
				script_args_storage.push_back(arg);
				continue;
			}

			if (arg == "--")
			{
				is_parsing_options = false;
				continue;
			}
			if (arg.starts_with("-"))
			{
				if (arg == "-h" || arg == "--help")
				{
					showHelp(programPath);
					return 0;
				} else if (arg == "-v" || arg == "--version") {
					std::println(PHASOR_VERSION_STRING);
					return 0;
				} else if (arg == "--verbose")
				{
					verbose = true;
				} else if (arg.starts_with("-D=") || arg.starts_with("--define="))
				{
					Phasor::string values = arg.substr(arg.find('=') + 1);
					std::stringstream ss(values);
					std::string item;
					while (std::getline(ss, item, ','))
					{
						if (!item.empty())
							defines_raw.push_back(item);
					}
				}
				else if (arg == "-D" || arg == "--define")
				{
					if (i + 1 >= argc)
					{
						std::println(std::cerr, "Error: -D/--define requires a NAME[=VALUE] argument");
						return 1;
					}
					Phasor::string values = argv[++i];
					std::stringstream ss(values);
					std::string item;
					while (std::getline(ss, item, ','))
					{
						if (!item.empty())
							defines_raw.push_back(item);
					}
				}
				else if (arg == "-c" || arg == "--command")
				{
					if (i + 1 >= argc)
					{
						std::println(std::cerr, "Error: -c/--command requires a script string argument");
						return 1;
					}
					command_script = argv[++i];
					has_command = true;
				} else {
					std::println(std::cerr, "Error: Unknown runtime option '{}'. Use -- to separate script arguments.", arg);
					return 1;
				}
			} else {
				if (!has_command && file_script.empty())
				{
					file_script = arg;
				} else {
					script_args_storage.push_back(arg);
				}
			}
		}

		bool        has_pipe = false;
		Phasor::string piped_script;
		if (!IS_TERMINAL)
		{
			piped_script = readStdin();
			if (!piped_script.empty())
			{
				has_pipe = true;
			}
		}

		bool has_file = !file_script.empty();

		int sources_count = (has_command ? 1 : 0) + (has_pipe ? 1 : 0) + (has_file ? 1 : 0);
		if (sources_count > 1)
		{
			std::println(std::cerr, "Error: Conflicting inputs. Cannot combine -c, piped input, and file input together.");
			return 1;
		}

		Phasor::string arg0 = "default.phs";
		if (has_file)
		{
			arg0 = file_script;
		}

		std::vector<Phasor::string> script_args_strings;
		script_args_strings.push_back(arg0);
		for (const auto &arg : script_args_storage)
		{
			script_args_strings.push_back(arg);
		}

		std::vector<char *> scriptArgv;
		scriptArgv.reserve(script_args_strings.size());
		for (auto &s : script_args_strings)
		{
			scriptArgv.push_back(s.data());
		}

		int    scriptArgc = static_cast<int>(scriptArgv.size());
		char **scriptArgvPtr = scriptArgv.data();
		const Phasor::Defines defines =
		    Phasor::resolveDefines(defines_raw, false);

		if (has_command)
		{
			auto vm = createVm(scriptArgc, scriptArgvPtr);
			int ret = runSourceString(command_script, *vm, includePaths, "", verbose, defines);
			return ret;
		} else if (has_pipe) {
			auto vm = createVm(scriptArgc, scriptArgvPtr);
			int ret =runSourceString(piped_script, *vm, includePaths, "", verbose, defines);
			return ret;
		} else if (has_file) {
			const std::filesystem::path file = file_script.str();
			if (!std::filesystem::exists(file))
			{
				std::println(std::cerr, "File not found: {}", file.string());
				return 1;
			}

			const Phasor::string ext = file.extension().string();
#ifndef PHS_WINDOWED
			if (ext == ".phsw" || ext == ".phsbw")
			{
				std::println("Use phasorw for windowed scripts, or rename to .phs/.phsb");
				return 1;
			} else if (ext == ".phs")
#else
			if (ext == ".phsw" || ext == ".phs")
#endif
			{
				int ret = runScriptFile(file, scriptArgc, scriptArgvPtr, includePaths, verbose, defines);
				return ret;
			}
#ifndef PHS_WINDOWED
			else if (ext == ".phsb")
#else
			else if (ext == ".phsbw" || ext == ".phsb")
#endif
			{
				int ret = runBytecodeFile(file, scriptArgc, scriptArgvPtr, verbose);
				return ret;
			} else {
				std::println(std::cerr, "Unsupported extension: {}, see --help", ext);
				return 1;
			}
		} else {
			int ret = runRepl(includePaths, verbose, defines);
			return ret;
		}
	}
	catch (const std::exception &e)
	{
		std::println(std::cerr, "Error: {}", e.what());
		return 1;
	}
	return 1;
}
}