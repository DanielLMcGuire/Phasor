#pragma once

#include <string>
#include <vector>
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
	Compiler(int argc, char *argv[], char *envp[]);
	int run();

  private:
	struct Args
	{
		std::string inputFile;
		std::string outputFile;
		bool        showLogo = true;
		bool        verbose = false;
		bool        irMode = false;
		int         scriptArgc = 0;
		char      **scriptArgv = nullptr;
		char      **envp = nullptr;
	} m_args;

	void parseArguments(int argc, char *argv[]);
	void showHelp(const std::string &programName);

	int compileToBytecode();
	int compileToIR();
};

} // namespace Phasor
