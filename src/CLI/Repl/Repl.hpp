#pragma once

#include "../../Runtime/Value.hpp"
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

/* Forward-declare Phasor::VM so this header refers to the single VM type
   defined in src/Runtime/VM/VM.hpp. */
namespace Phasor { class VM; }

namespace Phasor
{

struct AppArgs
{
	int    scriptArgc = 0;
	char **scriptArgv = nullptr;
	char **envp = nullptr;
};

/**
 * @class Repl
 * @brief Read-Eval-Print Loop for Phasor Programming Language
 */
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