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

void showHelp(fs::path program = "phasor") {
    std::cout << "Phasor Programming Language\n";
    std::cout << "Usage: [RAWSCRIPT] |  " << program.stem() << " [SCRIPT, BYTECODE]\n";
    std::cout << "A. PIPE: <text> | phasor\n";
    std::cout << "B. JIT/BYTECODE:  phasor <file>\n";
    std::cout << "C. REPL: phasor\n\n";
    std::cout << "Example:\n"; 
#ifdef _WIN32
    std::cout << "A. CMD:  echo \"print(^\"Hello, World!^\")\" | phasor\n";
    std::cout << "A. PWSH: echo \"print(`\"Hello, World!`\")\" | phasor\n";
    std::cout << "B.       phasor hello.phs\n";
    std::cout << "C.       phasor" << std::endl;
#else
    std::cout << "A. echo \"print(\\\"Hello, World!\\\")\" | phasor\n";
    std::cout << "B. phasor hello.phs\n";
    std::cout << "C. phasor" << std::endl;
#endif
}

int main(int argc, char *argv[], char *envp[])
{
    try {
        if (!IS_TERMINAL) {
            const std::string source = readStdin();
            if (!source.empty()) {
                Phasor::ScriptingRuntime ScriptRT(argc, argv, envp);
                Phasor::Frontend::runScript(source);
                goto end;
            }
        }
        if (argc < 2) {
            Phasor::Repl Repl(argc, argv, envp);
            return Repl.run();
        }

        const fs::path program = argv[0];
        const fs::path file = argv[1];

        if (!fs::exists(file)) {
            const std::string raw = file.string();
            if (!raw.empty() && (raw.front() == '-' || raw.front() == '/')) {
                std::string m_path = raw;
                m_path.erase(0, m_path.find_first_not_of("-/"));
                if (m_path == "help" || m_path == "h" || m_path == "?" || m_path == "h" || m_path == "help") {
                    showHelp(program);
                    goto end;
                }
                std::cerr << "Invalid argument: " << m_path << "\n";
            } else 
                std::cerr << "File not found: " << raw << "\n";

            goto err;
        }


        const std::string ext = file.extension().string();

        if (ext == ".phs") {
            Phasor::ScriptingRuntime ScriptRT(argc, argv, envp);
            return ScriptRT.run();
        } else if (ext == ".phsb") {
            Phasor::BinaryRuntime BinRT(argc, argv, envp);
            return BinRT.run();
        } else if (ext == ".phir") {
            std::cout << "Phasor IR (.phir) compilation not yet implemented.\n";
            goto end;
        } else {
            std::cerr << "Unknown extension: " << ext << "\n";
            goto err;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

end:
    return 0;
err:
    return 1;
}