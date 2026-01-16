#include "../Runtime/VM/VM.hpp"
#include <string>

namespace Phasor
{

/**
 * @namespace Frontend
 * @brief Frontend namespace
 * @details Contains all the frontend functions
 */
namespace Frontend
{
/**
 * @brief Run a script
 * @param source The source code to run
 * @param vm The virtual machine to run the script on
 * @return The result of the script
 */
void runScript(const std::string &source, VM *vm = nullptr);

/**
 * @brief Run an REPL
 * @param vm The virtual machine to run the REPL on
 */
void runRepl(VM *vm = nullptr);
} // namespace Frontend

} // namespace Phasor