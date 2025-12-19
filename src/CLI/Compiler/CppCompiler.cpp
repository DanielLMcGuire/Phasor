#include "CppCompiler.hpp"
#include "../../Backend/Lexer/Lexer.hpp"
#include "../../Backend/Parser/Parser.hpp"
#include "../../Codegen/CodeGen.hpp"
#include "../../Codegen/Cpp/CppCodeGenerator.hpp"
#include "../../Codegen/IR/PhasorIR.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace Phasor
{

CppCompiler::CppCompiler(int argc, char *argv[])
{
	parseArguments(argc, argv);
}

int CppCompiler::run()
{
	if (!m_args.noLogo)
	{
		std::cout << "Phasor C++ Code Generator\n";
		std::cout << "Copyright (c) 2025 Daniel McGuire\n";
		std::cout << "\n";
	}
	if (m_args.showHelp)
	{
		showHelp("phasor-cpp");
		return 0;
	}

	if (m_args.inputFile.empty() && !(m_args.headerOnly || m_args.generateOnly))
	{
		std::cerr << "Error: No input file provided\n";
		std::cerr << "Use --help for usage information\n";
		return 1;
	}

	if (m_args.mainFile.empty() && !m_args.headerOnly)
	#ifdef _WIN32
		m_args.mainFile = "C:\\Program Files\\Phasor VM\\Development\\nativestub.cpp";
	#else
		m_args.mainFile = "/usr/local/share/phasor/dev/nativestub.cpp"
	#endif

	if (m_args.moduleName.empty())
	{
		std::filesystem::path inputPath(m_args.inputFile);
		m_args.moduleName = inputPath.stem().string();
	}

	// Default output file if not specified
	if (m_args.outputFile.empty())
	{
#ifdef _WIN32
		m_args.outputFile = m_args.moduleName + ".exe";
#else
		m_args.outputFile = m_args.moduleName;
#endif
	}

	if (m_args.verbose)
	{
		std::cout << "Input file: " << m_args.inputFile << "\n";
		std::cout << "Output file: " << m_args.outputFile << "\n";
		if (!m_args.moduleName.empty())
			std::cout << "Module name: " << m_args.moduleName << "\n";
	}

	if (m_args.headerOnly)
	{
		generateHeader(m_args.inputFile, m_args.outputFile);
		return 0;
	}

	if (m_args.generateOnly)
	{
		generateHeader(m_args.inputFile, m_args.moduleName + ".h");
		generateSource(m_args.moduleName + ".h", m_args.outputFile);
		return 0;
	}

	if (m_args.objectOnly)
	{
		generateHeader(m_args.inputFile, m_args.moduleName + ".h");
		generateSource(m_args.moduleName + ".h", m_args.moduleName + ".cpp");
		compileSource(m_args.moduleName + ".cpp", m_args.outputFile);
		return 0;

	}

	std::cout << "Generating wrapper...\n";

	if (generateHeader(m_args.inputFile, m_args.moduleName + ".h")) 
		std::cout << m_args.inputFile << " -> " << m_args.moduleName + ".h\n";
	else 
	{
		std::cerr << "Error: Could not generate header file\n";
		return 1;
	}

	if (generateSource(m_args.mainFile, m_args.moduleName + ".cpp")) 
		std::cout << m_args.mainFile.filename() << " -> " << m_args.moduleName + ".cpp\n\n";
	else 
	{
		std::cerr << "Could not generate source file\n";
		return 1;
	}

	std::cout << "Compiling...\n";
	std::cout << "[COMPILER] ";
	if (compileSource(m_args.moduleName + ".cpp", m_args.moduleName + ".obj")) 
	std::cout << m_args.moduleName + ".cpp -> " << m_args.moduleName + ".obj\n\n";
	else
	{
		std::cerr << "Could not compile program\n";
		return 1;
	}

	std::cout << "Linking...\n";
	std::cout << "[LINKER] ";
	if (linkObject(m_args.moduleName + ".obj", m_args.outputFile))
	std::cout << m_args.moduleName + ".obj -> " << m_args.outputFile << "\n";
	else
	{
		std::cerr << "Could not link program\n";
		return 1;
	}

	return 0;
}

bool CppCompiler::parseArguments(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];

		if (arg == "-h" || arg == "--help")
		{
			m_args.showHelp = true;
			return true;
		}
		else if (arg == "-v" || arg == "--verbose")
		{
			m_args.verbose = true;
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
		else if (arg == "-H" || arg == "--header-only")
		{
			m_args.headerOnly = true;
		}
		else if (arg == "-g" || arg == "--generate-only")
		{
			m_args.generateOnly = true;
		}
		else if (arg == "-O" || arg == "--object-only")
		{
			m_args.objectOnly = true;
		}
		else if (arg == "-m" || arg == "--module")
		{
			if (i + 1 < argc)
			{
				m_args.moduleName = argv[++i];
			}
			else
			{
				std::cerr << "Error: " << arg << " requires an argument\n";
				m_args.showHelp = true;
				return true;
			}
		}
		else if (arg == "-c" || arg == "--compiler")
		{
			if (i + 1 < argc)
			{
				m_args.compiler = argv[++i];
				if (m_args.compiler == "cl" && m_args.linker.empty())
				{
					m_args.linker = "link";
				}
				else if ((m_args.compiler == "clang" || m_args.compiler == "clang++") && m_args.linker.empty())
				{
					m_args.linker = "clang";
				}
				else if ((m_args.compiler == "gcc" || m_args.compiler == "g++") && m_args.linker.empty())
				{
					m_args.linker = "gcc";
				}
			}
			else
			{
				std::cerr << "Error: " << arg << " requires an argument\n";
				m_args.showHelp = true;
				return true;
			}
		}
		else if (arg == "-l" || arg == "--linker")
		{
			if (i + 1 < argc)
			{
				m_args.linker = argv[++i];
			}
			else
			{
				std::cerr << "Error: " << arg << " requires an argument\n";
				m_args.showHelp = true;
				return true;
			}
		}
		else if (arg == "-s" || arg == "--source")
		{
			m_args.mainFile = argv[++i];
		}
		else if (arg[0] == '-')
		{
			std::cerr << "Error: Unknown option: " << arg << "\n";
			m_args.showHelp = true;
			return true;
		}
		else
		{
			// First non-option argument is the input file
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

bool CppCompiler::showHelp(const std::string &programName)
{
	std::cout << "Phasor C++ Code Generator\n\n";
	std::cout << "Usage:\n";
	std::cout << "  " << programName << " [options] <input.phs>\n\n";
	std::cout << "Options:\n";
	std::cout << "  -c, --compiler <name>   Compiler to use (default: g++)\n";
	std::cout << "  -l, --linker <name>     Linker to use (default: g++)\n";
	std::cout << "  -s, --source <name>     The source file to compile with\n";
	std::cout << "  -o, --output <file>   Output file\n";
	std::cout << "  -m, --module <name>   Module name for generated code (default: input filename)\n";
	std::cout << "  -H, --header-only     Generate header file only\n";
	std::cout << "  -g, --generate-only   Generate source file only\n";
	std::cout << "  -O, --object-only     Generate and compile to object only\n";
	std::cout << "  -v, --verbose         Enable verbose output\n";
	std::cout << "  -h, --help            Show this help message\n\n";
	std::cout << "  -n, --nologo          Do not show banner\n\n";
	std::cout << "Description:\n";
	std::cout << "  Compiles Phasor source code to a header file that embeds the bytecode.\n";
	std::cout << "  The header should be included in CppRuntime_main.cpp and compiled\n";
	std::cout << "  with the runtime to create a standalone executable.\n\n";
	std::cout << "Example:\n";
	std::cout << "  " << programName << " program.phs -o program.cpp\n";
	return true;
}

bool CppCompiler::generateHeader(const std::filesystem::path &sourcePath, const std::filesystem::path &outputPath)
{
	try
	{
		Bytecode bytecode;
		if (sourcePath.extension() == ".phir")
		{
			PhasorIR phasorIR;
			bytecode = phasorIR.loadFromFile(sourcePath);
		} else {
		// Read source file
		if (m_args.verbose)
			std::cout << "Reading source file...\n";

		std::ifstream file(sourcePath);
		if (!file.is_open())
		{
			std::cerr << "Error: Could not open input file: " << sourcePath << "\n";
			return false;
		}

		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string source = buffer.str();
		file.close();

		// Lex
		if (m_args.verbose)
			std::cout << "Lexing...\n";

		Lexer lexer(source);
		auto  tokens = lexer.tokenize();

		// Parse
		if (m_args.verbose)
			std::cout << "Parsing...\n";

		Parser parser(tokens);
		auto   program = parser.parse();

		// Generate bytecode
		if (m_args.verbose)
			std::cout << "Generating bytecode...\n";

		CodeGenerator codegen;
		bytecode = codegen.generate(*program);
	    }

		if (bytecode.instructions.empty())
		{
			std::cerr << "Error: No instructions generated\n";
			return false;
		}

		if (m_args.verbose)
		{
			std::cout << "Bytecode statistics:\n";
			std::cout << "  Instructions: " << bytecode.instructions.size() << "\n";
			std::cout << "  Constants: " << bytecode.constants.size() << "\n";
			std::cout << "  Variables: " << bytecode.variables.size() << "\n";
			std::cout << "  Functions: " << bytecode.functionEntries.size() << "\n";
		}

		// Generate C++ code
		if (m_args.verbose)
			std::cout << "Generating C++ code...\n";

		CppCodeGenerator cppGen;
		bool             success = cppGen.generate(bytecode, outputPath, m_args.moduleName);

		if (!success)
		{
			std::cerr << "Error: Failed to generate C++ code\n";
			return false;
		}

		if (m_args.verbose)
			std::cout << "Successfully generated: " << outputPath << "\n";
	}
	catch (const std::exception &e)
	{
		std::cerr << "Compilation Error: " << e.what() << "\n";
		return false;
	}
	return true;
}

bool CppCompiler::generateSource(const std::filesystem::path &sourcePath, const std::filesystem::path &outputPath)
{
	std::ifstream file(sourcePath);
	if (!file.is_open())
	{
		std::cerr << "Error: Could not open input file: " << sourcePath << "\n";
		return false;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string source = buffer.str();
	file.close();

	std::ofstream outputFile(outputPath);
	if (!outputFile.is_open())
	{
		std::cerr << "Error: Could not open output file: " << outputPath << "\n";
		return false;
	}

	// Include the generated header file (which is the output filename with .h extension)
	std::filesystem::path headerPath = outputPath;
	headerPath.replace_extension(".h");

	outputFile << "#include \"" << headerPath.filename().string() << "\"\n";
	outputFile << source;
	outputFile.close();
	
	return true;
}

bool CppCompiler::compileSource(const std::filesystem::path &sourcePath, const std::filesystem::path &outputPath)
{
	std::vector<std::string> flags;
	if (m_args.compiler == "cl")
		flags = {"/std:c++20", "/Ox", "/D",    "NDEBUG", "/MD",     "/GL", "/Gy-",
		         "/GS-",       "/Gw", "/EHsc", "/WX-",   "/nologo", "/c",  ("/Fo" + outputPath.string())};
	else if (m_args.compiler == "g++" || m_args.compiler == "clang++")
		flags = {"-std=c++20",
		         "-O3",
		         "-DNDEBUG",
		         "-fPIC",
		         "-flto",
		         "-fno-function-sections",
		         "-fno-stack-protector",
		         "-fwhole-program",
		         "-fexceptions",
		         "-Wno-error",
		         "-c",
		         ("-o" + outputPath.string())};

	else
	{
		std::cerr << "Error: Unknown compiler: " << m_args.compiler << "\n";
		return false;
	}

	std::string command = m_args.compiler;
	for (const auto &flag : flags)
	{
		command += " " + flag;
	}

	command += " " + sourcePath.string();
	std::system(command.c_str());

	return true;
}

bool CppCompiler::linkObject(const std::filesystem::path &objectPath, const std::filesystem::path &outputPath)
{
	std::string command = m_args.linker;
	command += " " + objectPath.string();
	if (m_args.linker == "link")
		command += " /NOLOGO /LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO /out:" + outputPath.string();
	else if (m_args.linker == "ld" || m_args.linker == "clang++" || m_args.linker == "clang")
		command += "-flto -pthread -Wl,--gc-sections -o " + outputPath.string();
	else
	{
		std::cerr << "Error: Unknown linker: " << m_args.linker << "\n";
		return false;
	}
	return (std::system(command.c_str()) == 0);
}

} // namespace Phasor

