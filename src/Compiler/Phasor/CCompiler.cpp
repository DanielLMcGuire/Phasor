#include "CCompiler.hpp"
#include "../../Language/Phasor/Lexer/Lexer.hpp"
#include "../../Language/Phasor/Parser/Parser.hpp"
#include "../../Language/Phasor/Parser/PlatformDefines.hpp"
#include "../../Codegen/CodeGen.hpp"
#include "../../Codegen/C/CCodeGenerator.hpp"
#include "../../Codegen/IR/PhasorIR.hpp"
#include <version.h>
#include <filesystem>
#include <fstream>
#include <print>
#include <sstream>
#include <phs_dupenv.hpp>

namespace Phasor
{

CCompiler::CCompiler(int argc, char *argv[])
{
	parseArguments(argc, argv);
}

int CCompiler::run()
{
	if (m_args.showHelp)
	{
		showHelp("phasorcc");
		return 0;
	}

	if (m_args.inputFile.empty() && !(m_args.headerOnly || m_args.generateOnly))
	{
		std::println(std::cerr, "Error: No input file provided\nUse --help for usage information");
		return 1;
	}

	if (m_args.mainFile.empty() && !m_args.headerOnly)
	{
#ifdef _WIN32
		m_args.mainFile = R"(C:\Program Files\Phasor VM\Development\nativestub.c)";
#else
		m_args.mainFile = "/usr/local/share/phasor/dev/nativestub.c";
#endif
    }

	if (m_args.moduleName.empty())
	{
		std::filesystem::path inputPath(m_args.inputFile);
		m_args.moduleName = Phasor::string(inputPath.stem().string());
	}

	// Default output file if not specified
	if (m_args.outputFile.empty())
	{
#ifdef _WIN32
		m_args.outputFile = m_args.moduleName.str() + ".exe";
#else
		m_args.outputFile = m_args.moduleName;
#endif
	}

	if (m_args.verbose)
	{
		std::println("Input file: {}\nOutput file: {}", m_args.inputFile.string(), m_args.outputFile.string());
		if (!m_args.moduleName.empty())
		{
			std::println("Module name: {}", m_args.moduleName);
		}
	}

	if (m_args.headerOnly)
	{
		generateHeader(m_args.inputFile, m_args.outputFile);
		return 0;
	}

	if (m_args.generateOnly)
	{
		generateHeader(m_args.inputFile, m_args.moduleName.str() + ".h");
		generateSource(m_args.moduleName.str() + ".h", m_args.outputFile);
		return 0;
	}

	if (m_args.objectOnly)
	{
		generateHeader(m_args.inputFile, m_args.moduleName.str() + ".h");
		generateSource(m_args.moduleName.str() + ".h", m_args.moduleName.str() + ".c");
		compileSource(m_args.moduleName.str() + ".c", m_args.outputFile);
		return 0;
	}

	std::println("Generating wrapper...");

	if (generateHeader(m_args.inputFile, m_args.moduleName.str() + ".h"))
	{
		std::println("{} -> {}.h", m_args.inputFile.string(), m_args.moduleName);
	} else {
		std::println(std::cerr, "Error: Could not generate header file");
		return 1;
	}

	if (generateSource(m_args.mainFile, m_args.moduleName.str() + ".c"))
	{
		std::println("{} -> {}.c\n", m_args.mainFile.filename().string(), m_args.moduleName.str());
	} else {
		std::println(std::cerr, "Could not generate source file");
		return 1;
	}

	std::println("Compiling...");
	std::print("[COMPILER] ");
	if (compileSource(m_args.moduleName.str() + ".c", m_args.moduleName.str() + ".obj"))
	{
		std::println("{}.c -> {}.obj\n", m_args.moduleName, m_args.moduleName.str());
	} else {
		std::println(std::cerr, "Could not compile program");
		return 1;
	}

	std::println("Linking...");
	std::print("[LINKER] ");
	if (linkObject(m_args.moduleName.str() + ".obj", m_args.outputFile))
	{
		std::println("{}.obj -> {}", m_args.moduleName, m_args.outputFile.string());
	} else {
		std::println(std::cerr, "Could not link program");
		return 1;
	}

	return 0;
}

bool CCompiler::parseArguments(int argc, char *argv[])
{
	for (int i = 1; i < argc; i++)
	{
		Phasor::string arg = argv[i];

		if (arg == "-h" || arg == "--help")
		{
			m_args.showHelp = true;
			return true;
		}
		if (arg == "-v" || arg == "--verbose")
		{
			m_args.verbose = true;
		} else if (arg.starts_with("-i=") || arg.starts_with("-I=") || arg.starts_with("--include=")) {
			Phasor::string values = arg.substr(arg.find('=') + 1);
			std::stringstream ss(values);
			std::string item;
			while (std::getline(ss, item, ','))
			{
				if (!item.empty())
					m_args.includePaths.push_back(item);
			}
		} else if (arg == "-I" || arg == "--include") {
			if (i + 1 < argc)
			{
				Phasor::string values = argv[++i];
				std::stringstream ss(values);
				std::string item;
				while (std::getline(ss, item, ','))
				{
					if (!item.empty())
						m_args.includePaths.push_back(item);
				}
			} else {
				std::println(std::cerr, "Error: {} requires an argument", arg);
				m_args.showHelp = true;
				return true;
			}
		} else if (arg.starts_with("-D=") || arg.starts_with("--define=")) {
			Phasor::string values = arg.substr(arg.find('=') + 1);
			std::stringstream ss(values);
			std::string item;
			while (std::getline(ss, item, ','))
			{
				if (!item.empty())
					m_args.defines.push_back(item);
			}
		} else if (arg == "-D" || arg == "--define") {
			if (i + 1 < argc)
			{
				Phasor::string values = argv[++i];
				std::stringstream ss(values);
				std::string item;
				while (std::getline(ss, item, ','))
				{
					if (!item.empty())
						m_args.defines.push_back(item);
				}
			} else
			{
				std::println(std::cerr, "Error: {} requires an argument", arg);
				m_args.showHelp = true;
				return true;
			}
		} else if (arg == "-o" || arg == "--output") {
			if (i + 1 < argc)
			{
				m_args.outputFile = argv[++i];
			} else
			{
				std::println(std::cerr, "Error: {} requires an argument", arg);
				m_args.showHelp = true;
				return true;
			}
		} else if (arg == "-H" || arg == "--header-only") {
			m_args.headerOnly = true;
		} else if (arg == "-g" || arg == "--generate-only") {
			m_args.generateOnly = true;
		} else if (arg == "-O" || arg == "--object-only") {
			m_args.objectOnly = true;
		} else if (arg == "-m" || arg == "--module") {
			if (i + 1 < argc)
			{
				m_args.moduleName = argv[++i];
			} else {
				std::println(std::cerr, "Error: {} requires an argument", arg);
				m_args.showHelp = true;
				return true;
			}
		} else if (arg == "-c" || arg == "--compiler")
		{
			if (i + 1 < argc)
			{
				m_args.compiler = argv[++i];
				if (m_args.compiler == "cl" && m_args.linker.empty())
				{
					m_args.linker = "link";
				}
				else if ((m_args.compiler == "clang" || m_args.compiler == "clang") && m_args.linker.empty())
				{
					m_args.linker = "clang";
				}
				else if ((m_args.compiler == "gcc" || m_args.compiler == "gcc") && m_args.linker.empty())
				{
					m_args.linker = "gcc";
				}
			} else{
				std::println(std::cerr, "Error: {} requires an argument", arg);
				m_args.showHelp = true;
				return true;
			}
		} else if (arg == "-l" || arg == "--linker") {
			if (i + 1 < argc)
			{
				m_args.linker = argv[++i];
			} else
			{
				std::println(std::cerr, "Error: {} requires an argument", arg);
				m_args.showHelp = true;
				return true;
			}
		} else if (arg == "-s" || arg == "--source") {
			m_args.mainFile = argv[++i];
		} else if (arg[0] == '-') {
			std::println(std::cerr, "Error: Unknown option: {}", arg);
			m_args.showHelp = true;
			return true;
		} else {
			// First non-option argument is the input file
			if (m_args.inputFile.empty())
			{
				m_args.inputFile = arg.str();
			} else {
				std::println(std::cerr, "Error: Multiple input files specified");
				m_args.showHelp = true;
				return true;
			}
		}
	}

	std::vector<std::filesystem::path> finalPaths;

#ifdef PHASOR_DEFAULT_FIRST_PATH
	finalPaths.emplace_back(PHASOR_DEFAULT_FIRST_PATH);
#endif

	for (const auto& p : m_args.includePaths)
	{
		finalPaths.push_back(p);
	}

	Phasor::string includeDirs;
	if (dupenv_ret ret = Phasor::dupenv(includeDirs, "PHASOR_INCLUDE_PATH"); ret == dupenv_ret::Success)
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
	m_args.includePaths = std::move(finalPaths);

	return false;
}

bool CCompiler::showHelp(const Phasor::string &programName)
{
	std::println("Phasor CC Wrapper v{}\n"
	             "(C) 2026 Daniel McGuire - Licensed under Apache 2.0\n\n"
	             "Usage:\n"
	             "  {} [OPTIONS...] INPUT\n\n"
	             "Options:\n"
	             "  -c, --compiler NAME     Compiler to use (default: gcc)\n"
	             "  -l, --linker NAME       Linker to use (default: gcc)\n"
	             "  -s, --source NAME       The source file to compile with\n"
	             "  -o, --output FILE       Output file\n"
	             "  -m, --module NAME       Module name for generated code (default: input filename)\n"
	             "  -H, --header-only       Generate header file only\n"
	             "  -g, --generate-only     Generate source file only\n"
	             "  -O, --object-only       Generate and compile to object only\n"
	             "  -I, --include PATHS     Comma-separated list of include directories\n"
	             "  -D, --define DEFS       Comma-separated list of NAME or NAME=VALUE definitions\n"
	             "  -v, --verbose           Enable verbose output\n"
	             "  -h, --help              Show this help message\n"
	             "Examples:\n"
	             "  {} program.phs -o program.exe -c clang -l lld\n"
	             "  {} -O program.phs -o program.o -c clang\n"
	             "  {} -H program.phs -o program.h\n"
	             "  {} -g program.phs -o program.c\n"
				 "See also:\n"
				 "  phasor-help {} 1",
	             PHASOR_VERSION_STRING, programName, programName, programName, programName, programName, programName);
	return true;
}

bool CCompiler::generateHeader(const std::filesystem::path &sourcePath, const std::filesystem::path &outputPath) const
{
	try
	{
		Bytecode bytecode;
		if (sourcePath.extension() == ".phir")
		{
			bytecode = Phasor::PhasorIR::loadFromFile(sourcePath);
		} else {
			// Read source file
			if (m_args.verbose)
			{
				std::println("Reading source file...");
			}

			std::ifstream file(sourcePath);
			if (!file.is_open())
			{
				std::println(std::cerr, "Error: Could not open input file: {}", sourcePath.string());
				return false;
			}

			std::stringstream buffer;
			buffer << file.rdbuf();
			Phasor::string source = buffer.str();
			file.close();

			// Lex
			if (m_args.verbose)
			{
				std::println("Lexing...");
			}

			Lexer lexer(source);
			auto  tokens = lexer.tokenize();

			// Parse
			if (m_args.verbose)
			{
				std::println("Parsing...");
			}

			Parser parser(tokens, sourcePath);
			parser.setIncludePaths(m_args.includePaths);
			parser.setDefines(Phasor::resolveDefines(m_args.defines, true));
			auto   program = parser.parse();

			if (m_args.verbose)
			{
				std::println("Generating bytecode...");
			}

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
			             "  Functions: {}",
			             bytecode.instructions.size(), bytecode.constants.size(), bytecode.variables.size(),
			             bytecode.functionEntries.size());
		}

		// Generate C code
		if (m_args.verbose)
		{
			std::println("Generating C code...");
		}

		CCodeGenerator cGen;
		bool             success = cGen.generate(bytecode, outputPath, m_args.moduleName);

		if (!success)
		{
			std::println(std::cerr, "Error: Failed to generate C code");
			return false;
		}

		if (m_args.verbose)
		{
			std::println("Successfully generated: {}", outputPath.string());
		}
	} catch (const std::exception &e) {
		std::println(std::cerr, "Compilation Error: {}", e.what());
		return false;
	}
	return true;
}

bool CCompiler::generateSource(const std::filesystem::path &sourcePath, const std::filesystem::path &outputPath)
{
	std::ifstream file(sourcePath);
	if (!file.is_open())
	{
		std::println(std::cerr, "Error: Could not open input file: {}", sourcePath.string());
		return false;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	Phasor::string source = buffer.str();
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

bool CCompiler::compileSource(const std::filesystem::path &sourcePath, const std::filesystem::path &outputPath)
{
	std::vector<Phasor::string> flags;
	if (m_args.compiler == "cl")
	{
		flags = {"/std:c++20", "/Ox", "/D", "NDEBUG", "/MD", "/GL", "/Gy-",
				"/GS-", "/Gw", "/EHsc", "/WX-", "/nologo", "/c",
				("/Fo" + outputPath.string())};
	}
	else if (m_args.compiler == "gcc" || m_args.compiler == "clang")
	{
		flags = {
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
	} else {
		std::println(std::cerr, "Error: Unknown compiler: {}", m_args.compiler);
		return false;
	}

	Phasor::string command = m_args.compiler;
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

bool CCompiler::linkObject(const std::filesystem::path &objectPath, const std::filesystem::path &outputPath)
{
	Phasor::string command = m_args.linker;
	command += " " + objectPath.string();
	if (m_args.linker == "link")
	{
		command += " /NOLOGO /LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO /out:" + outputPath.string();
	} else if (m_args.linker == "ld" || m_args.linker == "clang" || m_args.linker == "clang") {
		command += "-flto -pthread -Wl,--gc-sections -o " + outputPath.string();
	} else {
		std::println(std::cerr, "Error: Unknown linker: {}", m_args.linker);
		return false;
	}
	return (std::system(command.c_str()) == 0);
}

} // namespace Phasor
