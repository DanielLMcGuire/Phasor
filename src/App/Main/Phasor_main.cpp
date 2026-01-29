#include "../../CLI/Repl/Repl.hpp"
#include "../../CLI/Runtime/ScriptingRuntime.hpp"
#include "../../Frontend/Frontend.hpp"
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <io.h>
#define IS_TERMINAL _isatty(_fileno(stdin))
#else
#include <unistd.h>
#define IS_TERMINAL isatty(fileno(stdin))
#endif

/**
 * @brief Reads all content from stdin until EOF (piped input)
 */
std::string readStdin() {
    std::string content;
    std::string line;
    while (std::getline(std::cin, line)) {
        content += line + "\n";
    }
    return content;
}

int main(int argc, char *argv[], char *envp[])
{
    try
    {
        if (!IS_TERMINAL) {
            std::string source = readStdin();
            if (!source.empty()) {
                Phasor::ScriptingRuntime ScriptRT(argc, argv, envp);
                Phasor::Frontend::runScript(source); 
                return 0;
            }
        }

        if (argc < 2) {
            Phasor::Repl Repl(argc, argv, envp);
            return Repl.run();
        }

        Phasor::ScriptingRuntime ScriptRT(argc, argv, envp);
        return ScriptRT.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}