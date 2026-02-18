#include "Compiler.hpp"
#include "../../Language/Phasor/Lexer/Lexer.hpp"
#include "../../Language/Phasor/Parser/Parser.hpp"
#include "../../Codegen/Bytecode/BytecodeSerializer.hpp"
#include "../../Codegen/CodeGen.hpp"
#include "../../Codegen/IR/PhasorIR.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace Phasor
{

Compiler::Compiler(int argc, char *argv[], char *envp[])
{
	m_args.envp = envp;
	parseArguments(argc, argv);
}

int Compiler::run()
{
	if (m_args.showLogo)
		std::cout << "Phasor Compiler\n(C) 2026 Daniel McGuire\n\n";
	if (m_args.inputFile.empty())
	{
		std::cerr << "Error: No input file provided\n";
		return 1;
	}

	if (m_args.irMode)
		return compileToIR();

	return compileToBytecode();
}

int Compiler::compileToBytecode()
{
	if (std::filesystem::path(m_args.inputFile).extension() == ".phsb")
	{
		std::cerr << "Error: Cannot compile a bytecode file\n";
		return 1;
	}

	std::ifstream file(m_args.inputFile);
	if (!file.is_open())
	{
		std::cerr << "Could not open file: " << m_args.inputFile << "\n";
		return 1;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string source = buffer.str();

	try
	{
		Lexer         lexer(source);
		Parser        parser(lexer.tokenize());
		auto          program = parser.parse();
		CodeGenerator codegen;
		auto          bytecode = codegen.generate(*program);

		if (m_args.outputFile.empty())
		{
			m_args.outputFile = m_args.inputFile;
			std::filesystem::path path(m_args.outputFile);
			path.replace_extension(".phsb");
			m_args.outputFile = path.string();
		}

		BytecodeSerializer serializer;
		if (!serializer.saveToFile(bytecode, m_args.outputFile))
		{
			std::cerr << "Failed to save bytecode to: " << m_args.outputFile << "\n";
			return 1;
		}

		if (m_args.showLogo)
			std::cout << "Compiled successfully: " << m_args.inputFile << " -> " << m_args.outputFile << "\n";
		return 0;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Compilation Error: " << e.what() << "\n";
		return 1;
	}
}

int Compiler::compileToIR()
{
	if (std::filesystem::path(m_args.inputFile).extension() == ".phir")
	{
		std::cerr << "Error: Cannot compile a Phasor IR file\n";
		return 1;
	}

	std::ifstream file(m_args.inputFile);
	if (!file.is_open())
	{
		std::cerr << "Could not open file: " << m_args.inputFile << "\n";
		return 1;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string source = buffer.str();

	try
	{
		Lexer         lexer(source);
		Parser        parser(lexer.tokenize());
		auto          program = parser.parse();
		CodeGenerator codegen;
		auto          bytecode = codegen.generate(*program);

		if (m_args.outputFile.empty())
		{
			m_args.outputFile = m_args.inputFile;
			std::filesystem::path path(m_args.outputFile);
			path.replace_extension(".phir");
			m_args.outputFile = path.string();
		}

		if (!PhasorIR::saveToFile(bytecode, m_args.outputFile))
		{
			std::cerr << "Failed to save Phasor IR to: " << m_args.outputFile << "\n";
			return 1;
		}

		std::cout << "Compiled successfully to IR: " << m_args.inputFile << " -> " << m_args.outputFile << "\n";
		return 0;
	}
	catch (const std::exception &e)
	{
		std::cerr << "Compilation Error: " << e.what() << "\n";
		return 1;
	}
}

void Compiler::parseArguments(int argc, char *argv[])
{
	int defaultArgLocation = 1;
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];

		if (arg == "-v" || arg == "--verbose")
		{
			m_args.verbose = true;
		}
		else if (arg == "--no-logo")
		{
			m_args.showLogo = false;
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
				exit(1);
			}
		}
		else if (arg == "-i" || arg == "--ir")
		{
			m_args.irMode = true;
		}
		else if (arg == "-h" || arg == "--help")
		{
			showHelp(argv[0]);
			exit(0);
		}
		else
		{
			defaultArgLocation = i;
			m_args.inputFile = arg;
			break; // Stop parsing after finding the input file
		}
	}
	m_args.scriptArgv = argv + defaultArgLocation;
	m_args.scriptArgc = argc - defaultArgLocation;
}

void Compiler::showHelp(const std::string &programName)
{
	std::string filename = std::filesystem::path(programName).filename().string();
	std::cout << "Phasor Compiler\n\n";
	std::cout << "Usage:\n";
	std::cout << "  " << filename << " [options] <file.phs>\n\n";
	std::cout << "Options:\n";
	std::cout << "  -o, --output FILE   Specify output file\n";
	std::cout << "  -i, --ir            Compile to IR format (.phir) instead of bytecode\n";
	std::cout << "  -v, --verbose       Enable verbose output\n";
	std::cout << "  -h, --help          Show this help message\n";
}

} // namespace Phasor
