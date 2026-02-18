#pragma once

#include <string>
#include <vector>
/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/**
 * @class BinaryRuntime
 * @brief CLI wrapper for running Phasor bytecode binaries
 *
 * Loads and executes Phasor bytecode binaries (.phsb files).
 */
class BinaryRuntime
{
  public:
	BinaryRuntime(int argc, char *argv[], char *envp[]);
	int run();

  private:
	struct Args
	{
		std::string inputFile;
		bool        verbose = false;
		int         scriptArgc = 0;
		char      **scriptArgv = nullptr;
		char      **envp = nullptr;
	} m_args;

	void parseArguments(int argc, char *argv[]);
	void showHelp(const std::string &programName);
};

} // namespace Phasor
