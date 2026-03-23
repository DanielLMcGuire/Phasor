#include "CppCompiler.hpp"
#include "../../Language/Phasor/Lexer/Lexer.hpp"
#include "../../Language/Phasor/Parser/Parser.hpp"
#include "../../Codegen/CodeGen.hpp"
#include "../../Codegen/Cpp/CppCodeGenerator.hpp"
#include "../../Codegen/IR/PhasorIR.hpp"
#include <filesystem>
#include <fstream>
#include <print>
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
		std::println("Phasor C++ Code Generator\nCopyright (c) 2026 Daniel McGuire\n");
	
	if (m_args.showHelp)
	{
		showHelp("phasornative");
		return 0;
	}

	if (m_args.inputFile.empty() && !(m_args.headerOnly || m_args.generateOnly))
	{
		std::println(std::cerr, "Error: No input file provided\nUse --help for usage information");
		return 1;
	}

	if (m_args.mainFile.empty() && !m_args.headerOnly)
#ifdef _WIN32
		m_args.mainFile = "C:\\Program Files\\Phasor VM\\Development\\nativestub.cpp";
#else
		m_args.mainFile = "/usr/local/share/phasor/dev/nativestub.cpp";
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
		std::println("Input file: {}\nOutput file: {}", m_args.inputFile.string(), m_args.outputFile.string());
		if (!m_args.moduleName.empty())
			std::println("Module name: {}", m_args.moduleName);
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

	std::println("Generating wrapper...");

	if (generateHeader(m_args.inputFile, m_args.moduleName + ".h"))
		std::println("{} -> {}.h", m_args.inputFile.string(), m_args.moduleName);
	else
	{
		std::println(std::cerr, "Error: Could not generate header file");
		return 1;
	}

	if (generateSource(m_args.mainFile, m_args.moduleName + ".cpp"))
		std::println("{} -> {}.cpp\n", m_args.mainFile.filename().string(), m_args.moduleName);
	else
	{
		std::println(std::cerr, "Could not generate source file");
		return 1;
	}

	std::println("Compiling...");
	std::print("[COMPILER] ");
	if (compileSource(m_args.moduleName + ".cpp", m_args.moduleName + ".obj"))
		std::println("{}.cpp -> {}.obj\n", m_args.moduleName, m_args.moduleName);
	else
	{
		std::println(std::cerr, "Could not compile program");
		return 1;
	}

	std::println("Linking...");
	std::print("[LINKER] ");
	if (linkObject(m_args.moduleName + ".obj", m_args.outputFile))
		std::println("{}.obj -> {}", m_args.moduleName, m_args.outputFile.string());
	else
	{
		std::println(std::cerr, "Could not link program");
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
				std::println(std::cerr, "Error: {} requires an argument", arg);
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
				std::println(std::cerr, "Error: {} requires an argument", arg);
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
				std::println(std::cerr, "Error: {} requires an argument", arg);
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
				std::println(std::cerr, "Error: {} requires an argument", arg);
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
			std::println(std::cerr, "Error: Unknown option: {}", arg);
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
				std::println(std::cerr, "Error: Multiple input files specified");
				m_args.showHelp = true;
				return true;
			}
		}
	}
	return false;
}

bool CppCompiler::showHelp(const std::string &programName)
{
	std::println("Usage:\n"
	"  {} [options] <input.phs>\n\n"
	"Options:\n"
	"  -c, --compiler <name>   Compiler to use (default: g++)\n"
	"  -l, --linker <name>     Linker to use (default: g++)\n"
	"  -s, --source <name>     The source file to compile with\n"
	"  -o, --output <file>   Output file\n"
	"  -m, --module <name>   Module name for generated code (default: input filename)\n"
	"  -H, --header-only     Generate header file only\n"
	"  -g, --generate-only   Generate source file only\n"
	"  -O, --object-only     Generate and compile to object only\n"
	"  -v, --verbose         Enable verbose output\n"
	"  -h, --help            Show this help message\n"
	"  -n, --nologo          Do not show banner\n\n"
	"Example:\n"
	"  {} program.phs -o program.exe -c clang++ -l lld\n"
	"  {} -O program.phs -o program.obj -c clang++\n"
	"  {} -H program.phs -o program.hpp\n"
	"  {} -g program.phs -o program.cpp", programName, programName, programName, programName, programName);
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
		}
		else
		{
			// Read source file
			if (m_args.verbose)
				std::println("Reading source file...");

			std::ifstream file(sourcePath);
			if (!file.is_open())
			{
				std::println(std::cerr, "Error: Could not open input file: {}", sourcePath.string());
				return false;
			}

			std::stringstream buffer;
			buffer << file.rdbuf();
			std::string source = buffer.str();
			file.close();

			// Lex
			if (m_args.verbose)
				std::println("Lexing...");

			Lexer lexer(source);
			auto  tokens = lexer.tokenize();

			// Parse
			if (m_args.verbose)
				std::println("Parsing...");

			Parser parser(tokens, sourcePath);
			auto   program = parser.parse();

			// Generate bytecode
			if (m_args.verbose)
				std::println("Generating bytecode...");

			CodeGenerator codegen;
			bytecode = codegen.generate(*program);
		}

		if (bytecode.instructions.empty())
		{
			std::println(std::cerr, "Error: No instructions generated");
			return false;
		}

		if (m_args.verbose)
		{
			std::println("Bytecode statistics:\n"
			"  Instructions: {}\n"
			"  Constants: {}\n"
			"  Variables: {}\n"
			"  Functions: {}", bytecode.instructions.size(), bytecode.constants.size(),
			bytecode.variables.size(), bytecode.functionEntries.size());
		}

		// Generate C++ code
		if (m_args.verbose)
			std::println("Generating C++ code...");

		CppCodeGenerator cppGen;
		bool             success = cppGen.generate(bytecode, outputPath, m_args.moduleName);

		if (!success)
		{
			std::println(std::cerr, "Error: Failed to generate C++ code");
			return false;
		}

		if (m_args.verbose)
			std::println("Successfully generated: {}", outputPath.string());
	}
	catch (const std::exception &e)
	{
		std::println(std::cerr, "Compilation Error: {}", e.what());
		return false;
	}
	return true;
}

bool CppCompiler::generateSource(const std::filesystem::path &sourcePath, const std::filesystem::path &outputPath)
{
	std::ifstream file(sourcePath);
	if (!file.is_open())
	{
		std::println(std::cerr, "Error: Could not open input file: {}", sourcePath.string());
		return false;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string source = buffer.str();
	file.close();

	std::ofstream outputFile(outputPath);
	if (!outputFile.is_open())
	{
		std::println(std::cerr, "Error: Could not open output file: {}", outputPath.string());
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
		std::println(std::cerr, "Error: Unknown compiler: {}", m_args.compiler);
		return false;
	}

	std::string command = m_args.compiler;
	for (const auto &flag : flags)
	{
		command += " " + flag;
	}

	command += " " + sourcePath.string();
	if (std::system(command.c_str()) != 0)
	{
		std::println(std::cerr, "Error: Compilation failed");
		return false;
	}

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
		std::println(std::cerr, "Error: Unknown linker: {}", m_args.linker);
		return false;
	}
	return (std::system(command.c_str()) == 0);
}

} // namespace Phasor
