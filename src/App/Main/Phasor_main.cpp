#include "../../CLI/Repl/Repl.hpp"
#include "../../CLI/Runtime/ScriptingRuntime.hpp"
#include "../../CLI/Runtime/BinaryRuntime.hpp"
#include "../../Frontend/Frontend.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <iterator>

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
    while (std::getline(std::cin, line))
    {
        content += line + "\n";
    }
    return content;
}

namespace fs = std::filesystem;

int main(int argc, char *argv[], char *envp[])
{
    try {
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

        fs::path filePath = argv[1];
        std::string ext = filePath.extension().string();

        if (ext == ".phs") {
            Phasor::ScriptingRuntime ScriptRT(argc, argv, envp);
            return ScriptRT.run();
        } else if (ext == ".phsb") {
            Phasor::BinaryRuntime BinRT(argc, argv, envp);
            return BinRT.run();
        } else if (ext == ".phir") {
            std::cout << "Phasor IR (.phir) compilation not yet implemented.\n";
            return 0;
        } else {
            std::cerr << "Unknown extension: " << ext << "\n";
            return 1;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}