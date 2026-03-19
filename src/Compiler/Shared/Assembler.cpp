#include "../../Codegen/Bytecode/BytecodeSerializer.hpp"
#include "../../Codegen/IR/PhasorIR.hpp"

#include "Assembler.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace Phasor
{

Assembler::Assembler(int argc, char *argv[])
{
	parseArguments(argc, argv);
}

int Assembler::run()
{
	if (!m_args.noLogo)
	{
		std::cout << "Phasor Assembler\n";
		std::cout << "Copyright (c) 2026 Daniel McGuire\n";
		std::cout << "\n";
	}
	if (m_args.showHelp)
	{
		showHelp();
		return 0;
	}

	if (assembleBinary()) {
		if (!m_args.silent) std::cout << "Success! Output to " << m_args.outputFile << '\n';
		return 0;
	} else {
		std::cout << "Failed to assemble program!\n";
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
				std::cerr << "Error: " << arg << " requires an argument\n";
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

void Assembler::showHelp()
{
	std::cout << "Usage:\n";
	std::cout << "  " << m_args.program.stem().string() << " [options] <input.phsb>\n\n";
	std::cout << "Options:\n";
	std::cout << "  -o, --output <file>   Output file\n";
	std::cout << "  -h, --help            Show this help message\n";
	std::cout << "  -n, --nologo          Do not show banner\n";
	std::cout << "  -s, --silent          Do not print anything except errors (no stdout)\n\n";
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
