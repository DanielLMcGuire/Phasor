#pragma once

#include <PhasorString.hpp>
#include <vector>
#include <filesystem>
/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/**
 * @class Compiler
 * @brief CLI wrapper for bytecode generation from Phasor source
 *
 * Compiles Phasor source files to bytecode using the methods provided.
 */
class Compiler
{
  public:
	Compiler(int argc, char *argv[]);
	int run();

  private:
	struct Args
	{
		PhsString                        inputFile;
		PhsString                        outputFile;
		std::vector<std::filesystem::path> includePaths;
		std::vector<std::string>           defines;
		bool                               verbose = false;
		bool                               irMode = false;
		int                                scriptArgc = 0;
		char                             **scriptArgv = nullptr;
	} m_args;

	void        parseArguments(int argc, char *argv[]);
	static void showHelp(const PhsString &programName);

	int compileToBytecode();
	int compileToIR();
};

} // namespace Phasor