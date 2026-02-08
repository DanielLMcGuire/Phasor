
#include "interpreter.hpp"
#include <unordered_map>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

struct CommandHelp {
    std::string category;
    std::string description;
    std::string usage;
    std::string example;
};

static const std::unordered_map<std::string, CommandHelp>& getCommandHelpMap() {
    static std::unordered_map<std::string, CommandHelp> commandHelpMap = {
        // BUILT-IN
        {"exit",    {"BUILT-IN", "Exit the Pulsar interpreter", "exit", "exit"}},
        {"print",   {"BUILT-IN", "Print value", "print <value> ", "print \"Hello World\""}},
        {"println", {"BUILT-IN", "Print value with a newline", "println <value>", "println \"Hello World\""}},
        {"let",     {"BUILT-IN", "Declare and assign a variable", "let <variable> = <value>", "let x = 42"}},
        {"help",    {"BUILT-IN", "Show help information", "help [command_name]", "help print"}},
        {"if",      {"BUILT-IN", "Execute code block if condition is true", "if <condition> { <statements> }", "if x == 5 { println \"Found five!\" }"}},
        {"else",    {"BUILT-IN", "Execute alternative code block when if condition is false", "if <condition> { <statements> } else { <statements> }", "if age < 18 { println \"Minor\" } else { println \"Adult\" }"}},
        // FS
        {"cd",         {"FILESYSTEM", "Change directory", "cd <path>", "cd \"..\""}},
        {"pcd",        {"FILESYSTEM", "Print current directory", "pcd", "pcd"}},
        {"ls",         {"FILESYSTEM", "List directory contents", "ls [<path>]", "ls \"/home/user\""}},
        // FILE
        {"rl",   {"FILE", "Read a line from a file", "rl <filename> <line_number>", "rl \"data.txt\" 1"}},
        {"al", {"FILE", "Append a line to a file", "al <filename> <content>", "al \"data.txt\" \"New line\""}},
        {"cl", {"FILE", "Change a specific line in a file", "cl <filename> <line_number> <new_content>", "cl \"data.txt\" 1 \"Updated line\""}},
        {"fe",      {"FILE", "Check if a file exists", "fe <filename>", "fe \"data.txt\""}},
        {"rm",               {"FILE", "Delete a file", "rm <filename>", "rm \"data.txt\""}},
        {"mv",               {"FILE", "Move a file", "mv <old> <new>", "mv \"../old.txt\" \"./new.txt\""}},
        // STRING
        {"str_len",   {"STRING", "Get length of a string", "str_len <string>", "str_len \"Hello\""}},
        {"str_up", {"STRING", "Convert string to uppercase", "string_up <string>", "str_up \"hello world\""}},
        {"str_low", {"STRING", "Convert string to lowercase", "str_low <string>", "str_low \"HELLO WORLD\""}},
        {"str_merge",    {"STRING", "Merge two strings", "str_merge <string1> <string2>", "str_merge \"Hello\" \"World\""}},
        // MATH
        {"add",       {"MATH", "Add two integers", "add <int1> <int2>", "add 10 5"}},
        {"subtract",  {"MATH", "Subtract two integers", "subtract <int1> <int2>", "subtract 10 3"}},
        {"multiply",  {"MATH", "Multiply two integers", "multiply <int1> <int2>", "multiply 4 5"}},
        {"divide",    {"MATH", "Divide two integers", "divide <int1> <int2>", "divide 20 4"}},
        {"mod",   {"MATH", "Get modulus of two integers", "mod <int1> <int2>", "mod 10 3"}},
        {"exp",     {"MATH", "Raise an integer to a power", "exp <base> <exponent>", "exp 2 3"}},
        {"factor", {"MATH", "Calculate factorial of an integer", "factor <n>", "factor 5"}},
        {"abs", {"MATH", "Get absolute value of an integer", "abs <int>", "abs -42"}},
        {"max",  {"MATH", "Get maximum of two integers", "max <int1> <int2>", "max 10 20"}},
        {"min",  {"MATH", "Get minimum of two integers", "min <int1> <int2>", "min 10 20"}},
        {"sqrt",      {"MATH", "Calculate square root of an integer", "sqrt <n>", "sqrt 16"}},
        // SYS
        {"print_err", {"SYSTEM", "Print an error message", "print_err <message>", "print_err \"An error occurred\""}},
        {"sys_get_input", {"SYSTEM", "Get input from the user", "sys_get_input", "sys_get_input"}},
        {"sys_print_output", {"SYSTEM", "Print output to the console", "sys_print_output <message>", "sys_print_output \"Hello World\""}},
        {"sys_exec", {"SYSTEM", "Execute a system command", "sys_exec <command>", "sys_exec \"ls -l\""}},
        {"sys_wait", {"SYSTEM", "Wait for user to press Enter", "sys_wait", "sys_wait"}},
        {"clear", {"SYSTEM", "Clear the console screen", "clear", "clear"}}
    };
    return commandHelpMap;
}

void pulsar::interpreter::printCommandHelp(const std::string &command)
{
    std::cout << "\n";
    const auto& commandHelpMap = getCommandHelpMap();
    const auto& stdlibFuncs = getStandardLibraryFunctions();

    auto it = commandHelpMap.find(command);
    if (it != commandHelpMap.end()) {
        const auto& info = it->second;
        std::cout << command << " - " << info.description << "\n";
        std::cout << "Usage: " << info.usage << "\n";
        std::cout << "Example: " << info.example << "\n";
        return;
    }

    if (stdlibFuncs.find(command) != stdlibFuncs.end()) {
        // Fallback for stdlib commands not listed in map
        std::cout << command << " - Externally binded function\n";
        std::cout << "Usage: " << command << " <args>\n";
        std::cout << "Example: " << command << " ...\n";
        return;
    }

    std::cout << "Unknown command: " << command << "\n";
    std::cout << "Use 'help' to see all available commands.\n";
}

void pulsar::interpreter::printAllCommands()
{
    const auto& commandHelpMap = getCommandHelpMap();
    const auto& stdlibFuncs = getStandardLibraryFunctions();

    std::cout << "\n==== PULSAR LANGUAGE COMMANDS ====\n\n";

    std::unordered_map<std::string, CommandHelp> allCommands(commandHelpMap); // copy map to mutable

    for (const auto& [funcName, _] : stdlibFuncs) {
        if (allCommands.find(funcName) == allCommands.end()) {
            allCommands.emplace(funcName, CommandHelp{
                "MISC",
                "Standard library function (misc)",
                funcName + " <args>",
                funcName + " ..."
            });
        }
    }

    std::unordered_map<std::string, std::vector<std::string>> categorizedCommands;
    for (const auto& [cmd, info] : allCommands) {
        categorizedCommands[info.category].push_back(cmd);
    }
    
    std::vector<std::string> categories;
    for (const auto& [cat, _] : categorizedCommands) categories.push_back(cat);
    std::sort(categories.begin(), categories.end());
    auto it = std::find(categories.begin(), categories.end(), "BUILT-IN");
    if (it != categories.end()) {
        categories.erase(it);
        categories.insert(categories.begin(), "BUILT-IN");
    }

    for (const auto& category : categories) {
        std::cout << category << " COMMANDS:\n";

        auto& cmds = categorizedCommands[category];
        std::sort(cmds.begin(), cmds.end());

        for (const auto& cmd : cmds) {
            const auto& info = allCommands.at(cmd);
            int padding = std::max(1, 30 - (int)cmd.length());
            std::cout << "  " << cmd << std::string(padding, ' ') << "- " << info.description << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "Use 'help <command>' to get detailed info about a specific command.\n\n";
}
