#include "../../Codegen/Bytecode/BytecodeDeserializer.hpp"
#include "../../Codegen/IR/PhasorIR.hpp"

#include "Disassembler.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
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
		std::cout << "Phasor Decompiler\n";
		std::cout << "Copyright (c) 2026 Daniel McGuire\n";
		std::cout << "\n";
	}
	if (m_args.showHelp)
	{
		showHelp();
		return 0;
	}

	return (int)!decompileBinary();
}

bool Disassembler::parseArguments(int argc, char *argv[])
{
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
				std::cerr << "Error: " << arg << " requires an argument\n";
				m_args.showHelp = true;
				return true;
			}
		}
		else if (arg == "-n" || arg == "--nologo")
		{
			m_args.noLogo = true;
		}
		else if (arg[0] == '-')
		{
			std::cerr << "Error: Unknown option: " << arg << "\n";
			m_args.showHelp = true;
			return true;
		}
		else
		{
			if (m_args.inputFile.empty())
				m_args.inputFile = arg;
			else
			{
				std::cerr << "Error: Multiple input files specified\n";
				m_args.showHelp = true;
				return true;
			}
		}
	}
	return false;
}

const bool Disassembler::showHelp()
{
	std::cout << "Usage:\n";
	std::cout << "  " << m_args.programName << " [options] <input.phsb>\n\n";
	std::cout << "Options:\n";
	std::cout << "  -o, --output <file>   Output file\n";
	std::cout << "  -h, --help            Show this help message\n";
	std::cout << "  -n, --nologo          Do not show banner\n\n";
	return true;
}

const bool Disassembler::decompileBinary()
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
