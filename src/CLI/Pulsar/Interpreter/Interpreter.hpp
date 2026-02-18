#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../../Runtime/VM/VM.hpp"

namespace pulsar
{

/**
 * @class Interpreter
 * @brief CLI wrapper for running Pulsar scripts
 */
class Interpreter
{
  public:
	Interpreter(int argc, char *argv[], char *envp[]);
	int run();

  private:
	struct Args
	{
		std::filesystem::path inputFile;
		bool                  verbose = false;
		int                   scriptArgc = 0;
		char                **scriptArgv = nullptr;
		char                **envp = nullptr;
		std::filesystem::path program;
	} m_args;

	void parseArguments(int argc, char *argv[]);
	void showHelp();

	int  runSource();
	void runSourceString(const std::string &source, Phasor::VM &vm);

	std::unique_ptr<Phasor::VM> createVm();
};

} // namespace pulsar
