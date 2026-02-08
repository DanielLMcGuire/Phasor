#include "interpreter.hpp"

// This code should not be used in production.

void pulsar::interpreter::runREPL()
{
    std::cout << "==== Pulsar Language Interpreter ====\n";
    std::string line, buffer;
    
    while (true) {
        std::cout << (buffer.empty() ? "> " : "... ");
        std::getline(std::cin, line);

        if (line.empty() && buffer.empty()) continue;
        buffer += (buffer.empty() ? "" : "\n") + line;

        int round = 0, curly = 0, square = 0, angle = 0;
        for (char c : buffer) {
            switch (c) {
                case '(': round++; break;
                case ')': round--; break;
                case '{': curly++; break;
                case '}': curly--; break;
                case '[': square++; break;
                case ']': square--; break;
                case '<': angle++; break;
                case '>': angle--; break;
            }
        }

        if (round > 0 || curly > 0 || square > 0 || angle > 0) {
            continue;
        }

        try {
            auto commands = splitCommands(buffer);
            if (DEBUG) std::cout << "PULSAR: Got " << commands.size() << " commands\n";

            for (const auto& cmd : commands) {
                if (DEBUG) std::cout << "PULSAR: Processing command: '" << cmd << "'\n";
                auto tokens = tokenize(cmd);
                if (DEBUG) std::cout << "PULSAR: Tokenized into " << tokens.size() << " tokens\n";
                
                auto lexedTokens = lexer(tokens);
                if (DEBUG) std::cout << "PULSAR: Lexed into " << lexedTokens.size() << " tokens\n";
                
                if (DEBUG) std::cout << "PULSAR: Calling parser...\n";
                parser(lexedTokens);
                if (DEBUG) std::cout << "PULSAR: Parser completed\n";
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }

        buffer.clear();
    }
}


void pulsar::interpreter::runFromFile(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    if (DEBUG) std::cout << "PULSAR: Executing script from phasor: " << filename << std::endl;

    std::string line;
    int lineNumber = 0;
    std::string currentCommand;
    std::vector<char> openerStack;

    auto isOpener = [](char c) {
        return c == '(' || c == '{' || c == '[' || c == '<';
    };

    auto isCloser = [](char c) {
        return c == ')' || c == '}' || c == ']' || c == '>';
    };

    auto matches = [](char opener, char closer) {
        return (opener == '(' && closer == ')') ||
               (opener == '{' && closer == '}') ||
               (opener == '[' && closer == ']') ||
               (opener == '<' && closer == '>');
    };

    auto processCommand = [&](const std::string& cmd, int lineNum) {
        try {
            auto tokens = tokenize(cmd);
            auto lexedTokens = lexer(tokens);
            parser(lexedTokens);
        }
        catch (const std::exception& e) {
            std::cerr << "[Line " << lineNum << "] Error: " << e.what() << "\n";
        }
    };

    while (std::getline(file, line)) {
        ++lineNumber;
        if (line.empty()) continue;

        if (!currentCommand.empty()) currentCommand += "\n";
        currentCommand += line;

        for (char c : line) {
            if (isOpener(c)) {
                openerStack.push_back(c);
            } else if (isCloser(c)) {
                if (!openerStack.empty() && matches(openerStack.back(), c)) {
                    openerStack.pop_back();
                } else {
                    // Could be a syntax error but for now just ignore or handle later
                    std::cerr << "See line 114 of interpreter_io.cpp\n";
                }
            }
        }

        // If stack is empty we havve closed all blocks, so process the command
        if (openerStack.empty()) {
            processCommand(currentCommand, lineNumber);
            currentCommand.clear();
        }
        // Otherwise, keep appending lines until stack is empty
    }

    // Probably wont happen, but to be safe anyway
    if (!currentCommand.empty() && openerStack.empty()) {
        processCommand(currentCommand, lineNumber);
    } else if (!openerStack.empty()) {
        std::cerr << "Error: Unmatched opening symbol(s) at EOF\n";
    }
}
