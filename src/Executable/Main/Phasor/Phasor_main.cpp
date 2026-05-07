#include "../../../Runtime/Phasor/ScriptingRuntime.hpp"
#include "../../../Runtime/Shared/BinaryRuntime.hpp"
#include "../../../Frontend/Phasor/Frontend.hpp"
#include <print>
#include <string>
#include <vector>
#include <filesystem>
#include <iterator>

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
std::string readStdin()
{
	std::string content;
	std::string line;
	while (std::getline(std::cin, line))
	{
		content += line + "\n";
	}
	return content;
}

namespace fs = std::filesystem;

void showHelp(const fs::path &program = "phasor")
{
	const std::string programName = program.stem().string();
	std::println("Phasor Programming Language and Toolchain v{}\n"
	             "(C) 2026 Daniel McGuire - Licensed under Apache 2.0\n\n"
	             "Usage: [RAWSCRIPT] | {} [SCRIPT, BYTECODE]\n"
	             "A. PIPE:    <text> | {}\n"
	             "B. JIT/BYTECODE:     {} <file>\n"
	             "C. REPL:             {}\n\n"
	             "Example:",
	             PHASOR_VERSION_STRING, programName, programName, programName, programName);

#ifdef _WIN32
	std::println("A. CMD:  echo \"print(^\"Hi\\!\\n^\");\" | {}\n"
	             "A. PWSH: \"print(`\"Hi\\!\\n`\");\" | {}\n"
	             "B.       {} hello.phs\n"
	             "B.       {} hello.phsb",
	             programName, programName, programName, programName);
#else
	std::println("A. echo \"print(\\\"Hi\\!\\\\\\\n\\\");\" | {}\n"
	             "B. {} hello.phs\n"
	             "B. {} hello.phsb",
	             programName, programName, programName);
#endif
	std::println(R"(
Options:
    -h, --help     Show this help message and exit
    -v, --version  Show the version number and exit
    -c, --command  Run a raw script string)");
}

int main(int argc, char *argv[])
{
	try
	{
		if (!IS_TERMINAL)
		{
			const std::string source = readStdin();
			if (!source.empty())
			{
				return Phasor::Frontend::runScript(source, nullptr, "");
			}
		}
		if (argc < 2)
		{
			return Phasor::Frontend::runRepl();
		}

		const fs::path programPath = argv[0];
		const fs::path file = argv[1];

		if (!fs::exists(file))
		{
			const std::string raw = file.string();
			if (!raw.empty() && (raw.front() == '-' || raw.front() == '/'))
			{
				std::string m_path = raw;
				m_path.erase(0, m_path.find_first_not_of("-/"));
				if (m_path == "help" || m_path == "h" || m_path == "?" || m_path == "h" || m_path == "help")
				{
					showHelp(programPath);
					return 0;
				}
				else if (m_path == "version" || m_path == "v")
				{
					std::println(PHASOR_VERSION_STRING);
					return 0;
				}
				else if (m_path == "command" || m_path == "c")
				{
					Phasor::ScriptingRuntime ScriptRT(argc, argv);
					auto                     vm = ScriptRT.createVm();
					return ScriptRT.runSourceString(argv[2], *vm);
				}
				else
				{
					std::println(std::cerr, "Invalid argument: {}", m_path);
				}
			}
			else
			{
				std::println(std::cerr, "File not found: {}", raw);
			}
			return 1;
		}

		const std::string ext = file.extension().string();

		if (ext == ".phs")
		{
			Phasor::ScriptingRuntime ScriptRT(argc, argv);
			return ScriptRT.run();
		}
		else if (ext == ".phsb")
		{
			Phasor::BinaryRuntime BinRT(argc, argv);
			return BinRT.run();
		}
		else if (ext == ".phir")
		{
			std::println("Phasor IR (.phir) compilation not yet implemented.");
			return 0;
		}
		else
		{
			std::println(std::cerr, "Unknown extension: {}", ext);
			return 1;
		}
	}
	catch (const std::exception &e)
	{
		std::println(std::cerr, "Error: {}", e.what());
		return 1;
	}

	return 1;
}
