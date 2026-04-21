
#include "../../Codegen/Bytecode/BytecodeSerializer.hpp"
#include "../../Codegen/IR/PhasorIR.hpp"
#include <version.h>
#include "Assembler.hpp"

#include <filesystem>
#include <fstream>
#include <print>
#include <sstream>

namespace Phasor
{

Assembler::Assembler(int argc, char *argv[])
{
	parseArguments(argc, argv);
}

int Assembler::run()
{
	if (m_args.showHelp)
	{
		showHelp();
		return 0;
	}

	if (assembleBinary()) {
		if (!m_args.silent) std::println("Success! Output to {}", m_args.outputFile.string());
		return 0;
	} else {
		std::println("Failed to assemble program!");
		return 1;
	}
}

bool Assembler::parseArguments(int argc, char *argv[])
{
	m_args.program = std::filesystem::path(argv[0]);
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];

		if (arg == "-h" || arg == "--help")
		{
			m_args.showHelp = true;
			return true;
		}
		else if (arg == "-o" || arg == "--output")
		{
			if (i + 1 < argc)
			{
				m_args.outputFile = argv[++i];
			}
			else
			{
				std::println(std::cerr, "Error: {} requires an argument", arg);
				m_args.showHelp = true;
				return true;
			}
		}
		if (arg == "-s" || arg == "--silent")
		{
			m_args.silent = true;
		}
		else if (arg[0] == '-')
		{
			std::println(std::cerr, "Error: Unknown option: {}", arg);
			m_args.showHelp = true;
			return true;
		}
		else
		{
			if (m_args.inputFile.empty())
				m_args.inputFile = arg;
			else
			{
				std::println(std::cerr, "Error: Multiple input files specified");
				m_args.showHelp = true;
				return true;
			}
		}
	}
	return false;
}

void Assembler::showHelp()
{
	std::println("Phasor Assembler v{}\n"
	"(C) 2026 Daniel McGuire - Licensed under Apache 2.0\n\n"
	"Usage:\n"
	"  {} [options] <input.phsb>\n\n"
	"Options:\n"
	"  -o, --output <file>   Output file\n"
	"  -h, --help            Show this help message\n"
	"  -n, --nologo          Do not show banner\n"
	"  -s, --silent          Do not print anything except errors (no stdout)\n", PHASOR_VERSION_STRING, m_args.program.stem().string());
}

bool Assembler::assembleBinary()
{
	BytecodeSerializer bcSerializer{};
	PhasorIR             phir;

	if (m_args.outputFile.empty())
	{
		m_args.outputFile = m_args.inputFile;
		std::filesystem::path path(m_args.outputFile);
		path.replace_extension(".phsb");
		m_args.outputFile = path.string();
	}

	return bcSerializer.saveToFile(phir.loadFromFile(m_args.inputFile), m_args.outputFile);
}

} // namespace Phasor
