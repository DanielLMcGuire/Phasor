#include "../../Codegen/Bytecode/BytecodeDeserializer.hpp"
#include "../../Codegen/IR/PhasorIR.hpp"

#include "Disassembler.hpp"

#include <filesystem>
#include <fstream>
#include <print>
#include <sstream>

namespace Phasor
{

Disassembler::Disassembler(int argc, char *argv[])
{
	parseArguments(argc, argv);
}

int Disassembler::run()
{
	if (!m_args.noLogo)
	{
		std::println("Phasor Decompiler\nCopyright (c) 2026 Daniel McGuire");
	}
	if (m_args.showHelp)
	{
		showHelp();
		return 0;
	}

	if (decompileBinary()) {
		if (!m_args.silent) std::println("Success! Output to {}", m_args.outputFile.string());
		return 0;
	} else {
		std::println(std::cerr, "Failed to disassemble program!");
		return 1;
	}
}

bool Disassembler::parseArguments(int argc, char *argv[])
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
		else if (arg == "-n" || arg == "--nologo")
		{
			m_args.noLogo = true;
		}
		if (arg == "-s" || arg == "--silent")
		{
			m_args.noLogo = true;
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

void Disassembler::showHelp()
{
	std::println("Usage:\n" 
	"  {} [options] <input.phsb>"
	"Options:"
    "  -o, --output <file>   Output file"
    "  -h, --help            Show this help message"
    "  -n, --nologo          Do not show banner"
    "  -s, --silent          Do not print anything except errors (no stdout)", m_args.program.stem().string());
}

bool Disassembler::decompileBinary()
{
	BytecodeDeserializer bcDeserializer{};
	PhasorIR             phir;

	if (m_args.outputFile.empty())
	{
		m_args.outputFile = m_args.inputFile;
		std::filesystem::path path(m_args.outputFile);
		path.replace_extension(".phir");
		m_args.outputFile = path.string();
	}

	return phir.saveToFile(bcDeserializer.loadFromFile(m_args.inputFile), m_args.outputFile);
}

} // namespace Phasor
