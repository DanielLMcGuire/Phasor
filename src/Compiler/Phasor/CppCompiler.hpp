#pragma once

#include <string>
#include <filesystem>
/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/**
 * @class CppCompiler
 * @brief CLI wrapper for C++ code generation from Phasor source
 *
 * Compiles Phasor source files to C++ source files that embed bytecode
 * and link against the phasor-runtime DLL.
 */
class CppCompiler
{
  public:
	CppCompiler(int argc, char *argv[]);
	int run();

  private:
	struct Args
	{
		std::filesystem::path inputFile;
		std::filesystem::path outputFile;
		std::filesystem::path mainFile;
		std::string           moduleName;
		bool                  verbose = false;
		bool                  showHelp = false;
		std::string           compiler;
		std::string           linker;
		bool                  run = false;
		bool                  headerOnly = false;
		bool                  objectOnly = false;
		bool                  generateOnly = false;
		bool                  noLogo = false;
	} m_args;

	bool parseArguments(int argc, char *argv[]);
	bool showHelp(const std::string &programName);
	bool generateHeader(const std::filesystem::path &sourcePath, const std::filesystem::path &outputPath);
	bool generateSource(const std::filesystem::path &sourcePath, const std::filesystem::path &outputPath);
	bool compileSource(const std::filesystem::path &sourcePath, const std::filesystem::path &outputPath);
	bool linkObject(const std::filesystem::path &objectPath, const std::filesystem::path &outputPath);
};

} // namespace Phasor
