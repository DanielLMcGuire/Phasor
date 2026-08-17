#pragma once

#include <PhasorString.hpp>
#include <vector>
#include <filesystem>
/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/**
 * @class CCompiler
 * @brief CLI wrapper for C++ code generation from Phasor source
 *
 * Compiles Phasor source files to C++ source files that embed bytecode
 * and link against the phasor-runtime DLL.
 */
class CCompiler
{
  public:
	CCompiler(int argc, char *argv[]);
	int run();

  private:
	struct Args
	{
		std::filesystem::path              inputFile;
		std::filesystem::path              outputFile;
		std::filesystem::path              mainFile;
		std::vector<std::filesystem::path> includePaths;
		std::vector<std::string>           defines;
		PhsString                        moduleName;
		bool                               verbose = false;
		bool                               showHelp = false;
		PhsString                        compiler;
		PhsString                        linker;
		bool                               run = false;
		bool                               headerOnly = false;
		bool                               objectOnly = false;
		bool                               generateOnly = false;
	} m_args;

	bool parseArguments(int argc, char *argv[]);
	static bool showHelp(const PhsString &programName);
	bool generateHeader(const std::filesystem::path &sourcePath, const std::filesystem::path &outputPath) const;
	static bool generateSource(const std::filesystem::path &sourcePath, const std::filesystem::path &outputPath);
	bool compileSource(const std::filesystem::path &sourcePath, const std::filesystem::path &outputPath);
	bool linkObject(const std::filesystem::path &objectPath, const std::filesystem::path &outputPath);
};

} // namespace Phasor