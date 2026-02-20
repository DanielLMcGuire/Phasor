#include "../../Runtime/VM/VM.hpp"
#include <string>
/// @brief The Pulsar Scripting Language
namespace pulsar
{

/// @brief Frontend namespace
namespace Frontend
{
/**
 * @brief Run a script
 * @param source The source code to run
 * @param vm The virtual machine to run the script on
 * @return The result of the script
 */
int runScript(const std::string &source, Phasor::VM *vm = nullptr);

/**
 * @brief Run an REPL
 * @param vm The virtual machine to run the REPL on
 */
int runRepl(Phasor::VM *vm = nullptr);
} // namespace Frontend

} // namespace pulsar