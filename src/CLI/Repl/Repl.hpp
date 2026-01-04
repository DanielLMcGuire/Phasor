#pragma once

#include "../../Runtime/Value.hpp"
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

class VM; // Forward declaration

namespace Phasor
{

struct AppArgs
{
	int    scriptArgc = 0;
	char **scriptArgv = nullptr;
	char **envp = nullptr;
};

class Repl
{
  public:
	Repl(int argc, char *argv[], char *envp[]);
	int run();

  private:
	int         runRepl();
	static void runSourceString(const std::string &source, VM &vm);

	std::unique_ptr<VM> createVm();

	AppArgs m_args;
};

} // namespace Phasor