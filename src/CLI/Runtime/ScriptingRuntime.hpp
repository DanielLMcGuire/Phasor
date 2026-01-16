#pragma once

#include <memory>
#include <string>
#include <vector>

/* Forward-declare Phasor::VM so this header refers to the single VM type
   defined in src/Runtime/VM/VM.hpp. */
namespace Phasor { class VM; }

namespace Phasor
{

class ScriptingRuntime
{
  public:
	ScriptingRuntime(int argc, char *argv[], char *envp[]);
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

	int  runSource();
	void runSourceString(const std::string &source, VM &vm);

	std::unique_ptr<VM> createVm();
};

} // namespace Phasor
