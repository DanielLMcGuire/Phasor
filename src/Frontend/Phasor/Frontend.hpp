#include "../../Runtime/VM/VM.hpp"
#include <string>
/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/// @brief Frontend namespace
namespace Frontend
{
/**
 * @brief Run a script
 * @param source The source code to run
 * @param vm The virtual machine to run the script on
 * @param path The optional path of the source file
 * @return The result of the script
 */
int runScript(const std::string &source, VM *vm, const std::filesystem::path &path = "", bool verbose = false);
    /**
 * @brief Run an REPL
 * @param vm The virtual machine to run the REPL on
 */
int runRepl(VM *vm = nullptr, bool verbose = false);
} // namespace Frontend

} // namespace Phasor