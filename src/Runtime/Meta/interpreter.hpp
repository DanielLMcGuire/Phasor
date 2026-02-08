#pragma once

// PULSAR INTERPRETER IMPLEMENTATION -- interpreter/interpreter.h
// Author: Daniel McGuire 

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <regex>
#include <stdexcept>
#include <cctype>
#include <functional>

#if defined(_WIN32)
    #ifdef PULSAR_STD_BUILD
        #define PULSAR_EXT_API __declspec(dllexport)
    #else
        #define PULSAR_EXT_API __declspec(dllimport)
    #endif
#else
    #define PULSAR_EXT_API
#endif

namespace pulsar {
    class interpreter {
    public:
        static int DEBUG;

        static int runCommand(const std::string& command);
        static int runCommands(const std::vector<std::string>& commands);
        void runREPL();
        void runFromFile(const std::string& filename);

        enum class TokenType { 
            OPERATION,      // Built-in operations and standard library functions
            STRING,         // String literals
            INT,            // Integer literals
            IDENTIFIER,     // Variable names and identifiers
            ANGLE_LEFT,     // '<' for expression start (deprecated for conditionals)
            ANGLE_RIGHT,    // '>' for expression end (deprecated for conditionals)
            
            // Conditional and comparison tokens
            IF,             // if keyword
            ELSE,           // else keyword
            ENDIF,          // endif keyword (legacy - keeping for backward compatibility)
            
            // Block delimiters
            BLOCK_START,    // '{' for block start
            BLOCK_END,      // '}' for block end
            
            // C++ style operators
            EQUALS,         // ==
            NOT_EQUALS,     // !=
            LESS_THAN,      // <
            GREATER_THAN,   // >
            LESS_EQUAL,     // <=
            GREATER_EQUAL,  // >=
            LOGICAL_AND,    // &&
            LOGICAL_OR,     // ||
            LOGICAL_NOT,    // !
            FUNC,           // func keyword
            RETURN,         // return keyword
            WHILE,          // while keyword 
            COLON,          // : for type annotations
            COMMA,          // , for parameter separation
            LPAREN,         // ( 
            RPAREN,         // )
            
            // Boolean type
            BOOLEAN         // true/false
        };
        
        struct Token {
            std::string value;
            TokenType type;
        };

        inline Token makeToken(TokenType tokenType, const std::string& tokenValue) {
            return Token{tokenValue, tokenType};
        }

        static std::unordered_map<std::string, Token> symbolTable;

        using PhasorFunction = std::function<Token(const std::vector<Token>&)>;
        static std::unordered_map<std::string, std::function<Token(const std::vector<Token>&)>>& getStandardLibraryFunctions();

        static std::vector<std::string> splitCommands(const std::string& input);
        static std::vector<std::string> tokenize(const std::string& input);
        static std::vector<Token> lexer(const std::vector<std::string>& inputTokens);
        static Token parser(const std::vector<Token>& tokens);
        static std::vector<Token> resolveArguments(const std::vector<Token>& args);
    
        struct Parameter {
            std::string name;
            std::string type;
        };
        struct Function {
            std::string name;
            std::vector<Parameter> parameters;
            std::vector<Token> body;
            TokenType returnType;
        };
        static std::unordered_map<std::string, Function> functionTable;
        static std::unordered_map<std::string, Token> currentScope; 

        static Token callFunction(const std::string &name, const std::vector<Token> &args);
	    static Token getSymbol(const std::string &name);
    private:
        static Function parseFunction(const std::vector<Token>& tokens, size_t& pos);


        static bool evaluateComparison(const Token& left, const std::string& op, const Token& right);
        static Token evaluateCondition(const std::vector<Token>& conditionTokens);
        

        static void executeBlock(const std::vector<Token>& blockTokens);

        static bool isOperation(const std::string& input);
        static bool isInteger(const std::string& input);
        static bool isValidIdentifier(const std::string& input);
        static bool isStandardLibraryFunction(const std::string& input);

        static void printCommandHelp(const std::string& command);
        static void printAllCommands();
    };
}