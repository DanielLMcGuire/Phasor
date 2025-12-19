#pragma once

#include <string>
#include <vector>

namespace Phasor
{

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
