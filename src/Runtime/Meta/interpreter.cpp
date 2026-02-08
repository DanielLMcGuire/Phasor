// PULSAR SCRIPTING LANGUAGE IMPLEMENTATION -- interpreter/interpreter.cpp -- Part of the Phasor Programming Language Toolchain
// Author: Daniel McGuire 
// Purpose: A minimal scripting language to work effectively during Phasor runtime. 
// Phasor 0.3.158 alpha, aka "Pulsar"

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
#include "stdlib/stdlib.h"
#include "interpreter.hpp"
#include "api/api.h"

int pulsar::interpreter::DEBUG = 0;  
std::unordered_map<std::string, pulsar::interpreter::Function> pulsar::interpreter::functionTable;
std::unordered_map<std::string, pulsar::interpreter::Token>    pulsar::interpreter::currentScope;
std::unordered_map<std::string, pulsar::interpreter::Token>    pulsar::interpreter::symbolTable;

bool pulsar::interpreter::evaluateComparison(const Token& left, const std::string& op, const Token& right) {
    if (left.type == TokenType::STRING && right.type == TokenType::STRING) {
        if (op == "==") return left.value == right.value;
        if (op == "!=") return left.value != right.value;
        if (op == "<") return left.value < right.value;
        if (op == ">") return left.value > right.value;
        if (op == "<=") return left.value <= right.value;
        if (op == ">=") return left.value >= right.value;
    } else if (left.type == TokenType::INT && right.type == TokenType::INT) {
        int leftVal = std::stoi(left.value);
        int rightVal = std::stoi(right.value);
        
        if (op == "==") return leftVal == rightVal;
        if (op == "!=") return leftVal != rightVal;
        if (op == "<") return leftVal < rightVal;
        if (op == ">") return leftVal > rightVal;
        if (op == "<=") return leftVal <= rightVal;
        if (op == ">=") return leftVal >= rightVal;
    } else if (left.type == TokenType::BOOLEAN && right.type == TokenType::BOOLEAN) {
        bool leftVal = (left.value == "true");
        bool rightVal = (right.value == "true");
        
        if (op == "==") return leftVal == rightVal;
        if (op == "!=") return leftVal != rightVal;
    } else {
        if (op == "==") return false;
        if (op == "!=") return true;
    }
    
    throw std::runtime_error("Invalid comparison: " + left.value + " " + op + " " + right.value);
}

pulsar::interpreter::Token pulsar::interpreter::evaluateCondition(const std::vector<Token> &conditionTokens)
{
    if (conditionTokens.empty()) {
        throw std::runtime_error("Empty condition in if statement");
    }
    
    if (conditionTokens.size() == 1) {
        const Token& token = conditionTokens[0];
        if (token.type == TokenType::BOOLEAN) {
            return token;
        }
        if (token.type == TokenType::INT) {
            bool result = (std::stoi(token.value) != 0);
            return Token{result ? "true" : "false", TokenType::BOOLEAN};
        }
        if (token.type == TokenType::STRING) {
            bool result = (!token.value.empty() && token.value != "0" && token.value != "false");
            return Token{result ? "true" : "false", TokenType::BOOLEAN};
        }
    }
    
    if (conditionTokens.size() == 2 && conditionTokens[0].type == TokenType::LOGICAL_NOT) {
        Token subResult = evaluateCondition({conditionTokens[1]});
        bool result = !(subResult.value == "true");
        return Token{result ? "true" : "false", TokenType::BOOLEAN};
    }
    
    if (conditionTokens.size() == 3) {
        const Token& left = conditionTokens[0];
        const Token& op = conditionTokens[1];
        const Token& right = conditionTokens[2];
        
        if (op.type == TokenType::EQUALS || op.type == TokenType::NOT_EQUALS ||
            op.type == TokenType::LESS_THAN || op.type == TokenType::GREATER_THAN ||
            op.type == TokenType::LESS_EQUAL || op.type == TokenType::GREATER_EQUAL) {
            
            bool result = evaluateComparison(left, op.value, right);
            return Token{result ? "true" : "false", TokenType::BOOLEAN};
        }
    }
    
    for (size_t i = 1; i < conditionTokens.size() - 1; i += 2) {
        if (conditionTokens[i].type == TokenType::LOGICAL_AND) {
            std::vector<Token> leftTokens(conditionTokens.begin(), conditionTokens.begin() + i);
            std::vector<Token> rightTokens(conditionTokens.begin() + i + 1, conditionTokens.end());
            
            Token leftResult = evaluateCondition(leftTokens);
            if (leftResult.value == "false") {
                return Token{"false", TokenType::BOOLEAN}; 
            }
            
            Token rightResult = evaluateCondition(rightTokens);
            return rightResult;
        }
        else if (conditionTokens[i].type == TokenType::LOGICAL_OR) {
            std::vector<Token> leftTokens(conditionTokens.begin(), conditionTokens.begin() + i);
            std::vector<Token> rightTokens(conditionTokens.begin() + i + 1, conditionTokens.end());
            
            Token leftResult = evaluateCondition(leftTokens);
            if (leftResult.value == "true") {
                return Token{"true", TokenType::BOOLEAN}; 
            }
            
            Token rightResult = evaluateCondition(rightTokens);
            return rightResult;
        }
    }
    
    throw std::runtime_error("Invalid condition expression");
}

pulsar::interpreter::Function pulsar::interpreter::parseFunction(const std::vector<Token> &tokens, size_t &pos)
{
    Function func;
    
    if (pos + 1 >= tokens.size() || tokens[pos + 1].type != TokenType::IDENTIFIER) {
        throw std::runtime_error("func: Expected function name after 'func'");
    }
    func.name = tokens[pos + 1].value;
    pos += 2;
    
    if (pos < tokens.size() && tokens[pos].type == TokenType::IDENTIFIER) {
        std::string currentToken = tokens[pos].value;
        if (currentToken == "int") {
            func.returnType = TokenType::INT;
            pos++;
        } else if (currentToken == "string") {
            func.returnType = TokenType::STRING;
            pos++;
        } else if (currentToken == "bool") {
            func.returnType = TokenType::BOOLEAN;
            pos++;
        } else {
            func.returnType = TokenType::INT;
        }
    } else {
        func.returnType = TokenType::INT;
    }
    
    while (pos < tokens.size() && tokens[pos].type != TokenType::BLOCK_START) {
        if (tokens[pos].type == TokenType::IDENTIFIER) {
            std::string type = tokens[pos].value;
            
            if (pos + 2 >= tokens.size() || tokens[pos + 1].type != TokenType::COLON || 
                tokens[pos + 2].type != TokenType::IDENTIFIER) {
                throw std::runtime_error("func: Invalid parameter syntax. Expected 'type:name'");
            }
            
            std::string paramName = tokens[pos + 2].value;
            func.parameters.push_back({paramName, type});
            pos += 3;
        } else {
            throw std::runtime_error("func: Unexpected token '" + tokens[pos].value + 
                                   "' in parameter list. Expected 'type:name' or '{'");
        }
    }
    
    if (pos >= tokens.size() || tokens[pos].type != TokenType::BLOCK_START) {
        throw std::runtime_error("func: Expected '{' to start function body");
    }
    
    size_t bodyStart = pos + 1;
    int braceDepth = 1;
    pos++;
    
    while (pos < tokens.size() && braceDepth > 0) {
        if (tokens[pos].type == TokenType::BLOCK_START) {
            braceDepth++;
        } else if (tokens[pos].type == TokenType::BLOCK_END) {
            braceDepth--;
        }
        if (braceDepth > 0) {
            func.body.push_back(tokens[pos]);
        }
        pos++;
    }
    
    if (braceDepth != 0) {
        throw std::runtime_error("func: Missing closing '}' for function body");
    }
    
    return func;
}

pulsar::interpreter::Token pulsar::interpreter::getSymbol(const std::string &name)
{
    if (DEBUG)
        std::cout << "PULSAR: Phasor is accessing " << name << std::endl;
    auto it = symbolTable.find(name);
    if (it == symbolTable.end()) {
        throw std::runtime_error("Symbol '" + name + "' not found");
    }
    return it->second;
}

pulsar::interpreter::Token pulsar::interpreter::callFunction(const std::string &name, const std::vector<Token> &args)
{
    auto it = functionTable.find(name);
    if (it == functionTable.end()) {
        throw std::runtime_error("Unknown function '" + name + "'");
    }
    
    const Function& func = it->second;
    
    if (args.size() != func.parameters.size()) {
        throw std::runtime_error("Function '" + name + "' expects " + 
                                std::to_string(func.parameters.size()) + " arguments, got " + 
                                std::to_string(args.size()));
    }
    
    auto savedScope = symbolTable;

    for (size_t i = 0; i < func.parameters.size(); ++i) {
        const auto& param = func.parameters[i];
        const auto& arg = args[i];
        
        if ((param.type == "int" && arg.type != TokenType::INT) ||
            (param.type == "string" && arg.type != TokenType::STRING) ||
            (param.type == "bool" && arg.type != TokenType::BOOLEAN)) {
            throw std::runtime_error("Function '" + name + "': parameter '" + param.name + 
                                   "' expects type " + param.type);
        }
        
        symbolTable[param.name] = arg;
    }
    
    Token returnValue = {"0", TokenType::INT};
    
    try {
        executeBlock(func.body);
    } catch (const std::runtime_error& e) {
        std::string errorMsg = e.what();
        if (errorMsg.substr(0, 7) == "RETURN:") {
            std::string returnStr = errorMsg.substr(7);
            if (returnStr == "true" || returnStr == "false") {
                returnValue = {returnStr, TokenType::BOOLEAN};
            } else if (isInteger(returnStr)) {
                returnValue = {returnStr, TokenType::INT};
            } else {
                returnValue = {returnStr, TokenType::STRING};
            }
        } else {
            symbolTable = savedScope;
            throw;
        }
    }
    
    symbolTable = savedScope;
    
    return returnValue;
}

std::vector<pulsar::interpreter::Token> pulsar::interpreter::lexer(const std::vector<std::string> &inputTokens)
{
    if (DEBUG) {
        std::cout << "PULSAR: Lexer received " << inputTokens.size() << " tokens: ";
        for (const auto& tok : inputTokens) {
            std::cout << "[" << tok << "] ";
        }
        std::cout << std::endl;
    }
    
    std::vector<pulsar::interpreter::Token> lexedTokens;
    if (inputTokens.empty()) {
        throw std::runtime_error("Empty input - no tokens to process");
    }

    for (const auto& tok : inputTokens) {
        if (tok == "{") {
            lexedTokens.push_back({ tok, TokenType::BLOCK_START });
        }
        else if (tok == "}") {
            lexedTokens.push_back({ tok, TokenType::BLOCK_END });
        }
        else if (tok == "func") {
            lexedTokens.push_back({ tok, TokenType::FUNC });
        }
        else if (tok == "return") {
            lexedTokens.push_back({ tok, TokenType::RETURN });
        }
        else if (tok == "while") {
            lexedTokens.push_back({ tok, TokenType::WHILE });
        }
        else if (tok == ":") {
            lexedTokens.push_back({ tok, TokenType::COLON });
        }
        else if (tok == ",") {
            lexedTokens.push_back({ tok, TokenType::COMMA });
        }
        else if (tok == "(") {
            lexedTokens.push_back({ tok, TokenType::LPAREN });
        }
        else if (tok == ")") {
            lexedTokens.push_back({ tok, TokenType::RPAREN });
        }
        else if (tok == "if") {
            lexedTokens.push_back({ tok, TokenType::IF });
        }
        else if (tok == "else") {
            lexedTokens.push_back({ tok, TokenType::ELSE });
        }
        else if (tok == "endif") {
            lexedTokens.push_back({ tok, TokenType::ENDIF });
        }
        else if (tok == "true" || tok == "false") {
            lexedTokens.push_back({ tok, TokenType::BOOLEAN });
        }
        else if (tok == "==") {
            lexedTokens.push_back({ tok, TokenType::EQUALS });
        }
        else if (tok == "!=") {
            lexedTokens.push_back({ tok, TokenType::NOT_EQUALS });
        }
        else if (tok == "<=") {
            lexedTokens.push_back({ tok, TokenType::LESS_EQUAL });
        }
        else if (tok == ">=") {
            lexedTokens.push_back({ tok, TokenType::GREATER_EQUAL });
        }
        else if (tok == "&&") {
            lexedTokens.push_back({ tok, TokenType::LOGICAL_AND });
        }
        else if (tok == "||") {
            lexedTokens.push_back({ tok, TokenType::LOGICAL_OR });
        }
        else if (tok == "<") {
            lexedTokens.push_back({ tok, TokenType::ANGLE_LEFT });
        }
        else if (tok == ">") {
            lexedTokens.push_back({ tok, TokenType::ANGLE_RIGHT });
        }
        else if (tok == "!") {
            lexedTokens.push_back({ tok, TokenType::LOGICAL_NOT });
        }
        else if (isOperation(tok) || isStandardLibraryFunction(tok)) {
            lexedTokens.push_back({ tok, TokenType::OPERATION });
        }
        else if (isInteger(tok)) {
            lexedTokens.push_back({ tok, TokenType::INT });
        }
        else if (tok.size() >= 2 && tok.front() == '"' && tok.back() == '"') {
            std::string content = tok.substr(1, tok.size() - 2);
            std::string unescaped;
            for (size_t i = 0; i < content.size(); ++i) {
                if (content[i] == '\\' && i + 1 < content.size() && content[i + 1] == '"') {
                    unescaped += '"';
                    i++;
                } else {
                    unescaped += content[i];
                }
            }
            lexedTokens.push_back({ unescaped, TokenType::STRING });
        }
        else if (isValidIdentifier(tok) || tok == "=") {
            lexedTokens.push_back({ tok, TokenType::IDENTIFIER });
        }
        else {
            throw std::runtime_error("Unrecognized token: '" + tok + "' - check syntax");
        }
    }

    return lexedTokens;
}

void pulsar::interpreter::executeBlock(const std::vector<Token> &blockTokens)
{
    if (blockTokens.empty()) {
        return;
    }
    
    if (DEBUG) {
        std::cout << "PULSAR: Executing block with " << blockTokens.size() << " tokens" << std::endl;
    }
    
    std::vector<std::vector<Token>> statements;
    std::vector<Token> currentStatement;
    int angleDepth = 0;
    int braceDepth = 0;
    
    for (size_t i = 0; i < blockTokens.size(); ++i) {
        const auto& token = blockTokens[i];
        
        if (token.type == TokenType::ANGLE_LEFT || token.type == TokenType::LESS_THAN) {
            angleDepth++;
        } else if (token.type == TokenType::ANGLE_RIGHT || token.type == TokenType::GREATER_THAN) {
            angleDepth--;
        } else if (token.type == TokenType::BLOCK_START) {
            braceDepth++;
        } else if (token.type == TokenType::BLOCK_END) {
            braceDepth--;
        }
        
        currentStatement.push_back(token);
        
        if (i + 1 < blockTokens.size()) {
            const auto& nextToken = blockTokens[i + 1];
            
            if ((nextToken.type == TokenType::OPERATION || isOperation(nextToken.value)) && 
                !currentStatement.empty() && 
                angleDepth == 0 && 
                braceDepth == 0) {
                
                statements.push_back(currentStatement);
                currentStatement.clear();
            }
        }
    }
    
    if (!currentStatement.empty()) {
        statements.push_back(currentStatement);
    }
    
    for (const auto& statement : statements) {
        if (!statement.empty()) {
            if (DEBUG) {
                std::cout << "PULSAR: Executing statement with " << statement.size() << " tokens: ";
                for (const auto& tok : statement) {
                    std::cout << "[" << tok.value << "] ";
                }
                std::cout << std::endl;
            }
            
            try {
                parser(statement);
            } catch (const std::runtime_error& e) {
                std::string errorMsg = e.what();
                if (errorMsg.substr(0, 7) == "RETURN:") {
                    throw;
                } else {
                    throw std::runtime_error("Error in block execution: " + std::string(e.what()));
                }
            }
        }
    }
}

std::vector<std::string> pulsar::interpreter::splitCommands(const std::string &input)
{
    std::vector<std::string> commands;
    std::string current;
    bool inQuotes = false;
    bool escapeNext = false;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];
        
        if (escapeNext) {
            current += c;
            escapeNext = false;
            continue;
        }
        
        if (c == '\\' && inQuotes) {
            escapeNext = true;
            current += c;
            continue;
        }
        
        if (c == '"') {
            inQuotes = !inQuotes;
            current += c;
            continue;
        }
        
        if (c == ';' && !inQuotes) {
            if (!current.empty()) {
                size_t start = current.find_first_not_of(" \t\n\r");
                size_t end = current.find_last_not_of(" \t\n\r");
                if (start != std::string::npos) {
                    commands.push_back(current.substr(start, end - start + 1));
                }
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        size_t start = current.find_first_not_of(" \t\n\r");
        size_t end = current.find_last_not_of(" \t\n\r");
        if (start != std::string::npos) {
            commands.push_back(current.substr(start, end - start + 1));
        }
    }

    return commands;
}

std::vector<std::string> pulsar::interpreter::tokenize(const std::string &input)
{
    std::vector<std::string> tokens;
    std::string current;
    bool inQuotes = false;

    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];

        if (inQuotes) {
            current += c;
            if (c == '"' && (i == 0 || input[i - 1] != '\\')) {
                tokens.push_back(current);
                current.clear();
                inQuotes = false;
            }
        } else {
            if (c == ':' || c == ',' || c == '(' || c == ')') {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
                tokens.push_back(std::string(1, c));
            }
            if (c == '"') {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
                current += c;
                inQuotes = true;
            }
            else if (std::isspace(static_cast<unsigned char>(c))) {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            }
            else if (i < input.size() - 1) {
                std::string twoChar = std::string(1, c) + input[i + 1];
                if (twoChar == "==" || twoChar == "!=" || twoChar == "<=" ||
                    twoChar == ">=" || twoChar == "&&" || twoChar == "||") {
                    if (!current.empty()) {
                        tokens.push_back(current);
                        current.clear();
                    }
                    tokens.push_back(twoChar);
                    ++i;
                }
                else if (c == '=' || c == '<' || c == '>' || c == '!') {
                    if (!current.empty()) {
                        tokens.push_back(current);
                        current.clear();
                    }
                    tokens.push_back(std::string(1, c));
                }
                else {
                    current += c;
                }
            }
            else if (c == '=' || c == '<' || c == '>' || c == '!' || c == '{' || c == '}') {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
                tokens.push_back(std::string(1, c));
            }
            else {
                current += c;
            }
        }
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    if (inQuotes) {
        throw std::runtime_error("Unterminated string literal - missing closing quote");
    }

    return tokens;
}

bool pulsar::interpreter::isOperation(const std::string &input)
{
    static const std::unordered_set<std::string> ops = { "print", "println", "let" };
    return ops.find(input) != ops.end();
}

bool pulsar::interpreter::isInteger(const std::string &str)
{
    return std::regex_match(str, std::regex("^-?[0-9]+$"));
}

bool pulsar::interpreter::isValidIdentifier(const std::string &str)
{
    return std::regex_match(str, std::regex("^[a-zA-Z_][a-zA-Z0-9_]*$"));
}

bool pulsar::interpreter::isStandardLibraryFunction(const std::string &input)
{
    return (getStandardLibraryFunctions()).find(input) != (getStandardLibraryFunctions()).end();
}

void registerStandardLibraryFunction(const std::string& name, std::function<pulsar::interpreter::Token(const std::vector<pulsar::interpreter::Token> &)> func)
{
	(pulsar::interpreter::getStandardLibraryFunctions())[name] = func;
}

std::vector<pulsar::interpreter::Token> pulsar::interpreter::resolveArguments(
    const std::vector<pulsar::interpreter::Token> &args)
{
	std::vector<pulsar::interpreter::Token> resolved;
    size_t i = 0;
    
    while (i < args.size()) {
		if (args[i].type == pulsar::interpreter::TokenType::ANGLE_LEFT || 
            args[i].type == pulsar::interpreter::TokenType::LESS_THAN)
		{
            int depth = 1;
            size_t j = i + 1;
            
            while (j < args.size() && depth > 0) {
				if (args[j].type == pulsar::interpreter::TokenType::ANGLE_LEFT ||
				    args[j].type == pulsar::interpreter::TokenType::LESS_THAN)
				{
                    depth++;
				}
				else if (args[j].type == pulsar::interpreter::TokenType::ANGLE_RIGHT ||
				         args[j].type == pulsar::interpreter::TokenType::GREATER_THAN)
				{
                    depth--;
                }
                j++;
            }
            
            if (depth != 0) {
                throw std::runtime_error("Unmatched '<' bracket - missing closing '>' bracket");
            }

            std::vector<pulsar::interpreter::Token> subTokens(args.begin() + i + 1, args.begin() + j - 1);
			pulsar::interpreter::Token              subResult = pulsar::interpreter::parser(subTokens);

            resolved.push_back(subResult);
            i = j;
        } 
        else if (args[i].type == pulsar::interpreter::TokenType::IDENTIFIER)
		{
			auto it = pulsar::interpreter::symbolTable.find(args[i].value);
			if (it != pulsar::interpreter::symbolTable.end())
			{
                resolved.push_back(it->second);
            } else {
                throw std::runtime_error("Undefined variable '" + args[i].value + "' - variable must be declared with 'let' before use");
            }
            i++;
        } 
        else {
            resolved.push_back(args[i]);
            i++;
        }
    }
    
    return resolved;
}

pulsar::interpreter::Token pulsar::interpreter::parser(const std::vector<Token> &tokens)
{
    if (tokens.empty()) {
        throw std::runtime_error("No tokens to parse - empty expression");
    }

    const std::string& operation = tokens[0].value;
    
    if (operation == "exit") {
        std::cout << "Exiting Pulsar interpreter..." << std::endl;
        std::exit(0);
    }

    if (operation == "func") {
        size_t pos = 0;
        Function func = parseFunction(tokens, pos);
        functionTable[func.name] = func;
        
        if (DEBUG) {
            std::cout << "PULSAR: Function '" << func.name << "' declared with " 
                      << func.parameters.size() << " parameters" << std::endl;
        }
        
        return Token{"0", TokenType::INT};
    }

    if (operation == "let" && 
        (tokens.size() < 2 || tokens[1].type != TokenType::FUNC)) {
        
        if (tokens.size() < 4) {
            throw std::runtime_error("let: Incomplete variable declaration. "
                                   "Usage: let variable_name = value");
        }
        if (tokens[2].value != "=") {
            throw std::runtime_error("let: Expected '=' in variable declaration. "
                                   "Usage: let variable_name = value");
        }
        
        const auto& variableToken = tokens[1];
		if (variableToken.type != pulsar::interpreter::TokenType::IDENTIFIER)
		{
            throw std::runtime_error("let: Invalid variable name '" + variableToken.value + "'. "
                                   "Variable names must start with a letter or underscore and contain only letters, numbers, and underscores");
        }
        
		std::vector<pulsar::interpreter::Token> valueTokens(tokens.begin() + 3, tokens.end());
        auto resolvedArgs = resolveArguments(valueTokens);
        
        if (resolvedArgs.size() != 1) {
            throw std::runtime_error("let: Invalid value expression for variable '" + variableToken.value + "'. "
                                   "Expected a single value or expression");
        }
        
        const auto& valueToken = resolvedArgs[0];
		if (valueToken.type != pulsar::interpreter::TokenType::STRING && 
            valueToken.type != pulsar::interpreter::TokenType::INT &&
		    valueToken.type != pulsar::interpreter::TokenType::BOOLEAN)
		{
            throw std::runtime_error("let: Invalid value type for variable '" + variableToken.value + "'. "
                                   "Variables can only store strings, integers, or booleans");
        }
        
        pulsar::interpreter::symbolTable[variableToken.value] = valueToken;
        
        if (DEBUG) {
            std::cout << "PULSAR: Variable '" << variableToken.value << "' set to: ";
			if (valueToken.type == pulsar::interpreter::TokenType::STRING)
			{
                std::cout << "\"" << valueToken.value << "\" (STRING)";
			}
			else if (valueToken.type == pulsar::interpreter::TokenType::BOOLEAN)
			{
                std::cout << valueToken.value << " (BOOLEAN)";
            } else {
                std::cout << valueToken.value << " (INT)";
            }
            std::cout << std::endl;
        }
        
        return valueToken;
    }

    if (operation == "if") {
        if (tokens.size() < 2) {
            throw std::runtime_error("if: Missing condition. Usage: if <condition> { <statements> } [else { <statements> }]");
        }
        
        size_t conditionEnd = 1;
        size_t blockStart = tokens.size();
        size_t elsePos = tokens.size();
        
        for (size_t i = 1; i < tokens.size(); ++i) {
            if (tokens[i].type == TokenType::BLOCK_START) {
                conditionEnd = i;
                blockStart = i;
                break;
            }
        }
        
        if (blockStart == tokens.size()) {
            throw std::runtime_error("if: Missing opening '{' after condition");
        }
        
        size_t blockEnd = blockStart;
        int braceDepth = 0;
        for (size_t i = blockStart; i < tokens.size(); ++i) {
            if (tokens[i].type == TokenType::BLOCK_START) {
                braceDepth++;
            } else if (tokens[i].type == TokenType::BLOCK_END) {
                braceDepth--;
                if (braceDepth == 0) {
                    blockEnd = i;
                    break;
                }
            }
        }
        
        if (braceDepth != 0) {
            throw std::runtime_error("if: Missing closing '}' for if block");
        }
        
        if (blockEnd + 1 < tokens.size() && tokens[blockEnd + 1].type == TokenType::ELSE) {
            elsePos = blockEnd + 1;
        }
        
        std::vector<Token> conditionTokens(tokens.begin() + 1, tokens.begin() + conditionEnd);
        auto resolvedCondition = resolveArguments(conditionTokens);
        Token conditionResult = evaluateCondition(resolvedCondition);
        
        bool conditionTrue = (conditionResult.value == "true");
        
        if (DEBUG) {
            std::cout << "PULSAR: IF condition evaluated to: " << conditionResult.value << std::endl;
        }
        
        if (conditionTrue) {
            std::vector<Token> ifBlockTokens(tokens.begin() + blockStart + 1, tokens.begin() + blockEnd);
            executeBlock(ifBlockTokens);
        } else if (elsePos < tokens.size()) {
            size_t elseBlockStart = elsePos + 1;
            
            if (elseBlockStart < tokens.size() && tokens[elseBlockStart].type == TokenType::BLOCK_START) {
                size_t elseBlockEnd = elseBlockStart;
                int elseBraceDepth = 0;
                for (size_t i = elseBlockStart; i < tokens.size(); ++i) {
                    if (tokens[i].type == TokenType::BLOCK_START) {
                        elseBraceDepth++;
                    } else if (tokens[i].type == TokenType::BLOCK_END) {
                        elseBraceDepth--;
                        if (elseBraceDepth == 0) {
                            elseBlockEnd = i;
                            break;
                        }
                    }
                }
                
                if (elseBraceDepth != 0) {
                    throw std::runtime_error("if: Missing closing '}' for else block");
                }
                
                std::vector<Token> elseBlockTokens(tokens.begin() + elseBlockStart + 1, tokens.begin() + elseBlockEnd);
                executeBlock(elseBlockTokens);
            } else {
                throw std::runtime_error("if: Missing opening '{' after else");
            }
        }
        
        return Token{"0", TokenType::INT};
    }

    if (operation == "print" || operation == "println") {
        if (tokens.size() == 1) {
            if (operation == "println") {
                std::cout << std::endl;
            }
			return pulsar::interpreter::Token{"0", pulsar::interpreter::TokenType::INT};
        }
        
		std::vector<pulsar::interpreter::Token> args(tokens.begin() + 1, tokens.end());
        auto resolvedArgs = resolveArguments(args);
        
        for (const auto& token : resolvedArgs) {
			if (token.type == pulsar::interpreter::TokenType::STRING || 
                token.type == pulsar::interpreter::TokenType::INT ||
                token.type == pulsar::interpreter::TokenType::BOOLEAN)
			{
                std::cout << token.value;
            } else {
                throw std::runtime_error("print/println: Invalid token type in print statement. Only strings, integers, and booleans can be printed");
            }
        }
        
        if (operation == "println") {
            std::cout << std::endl;
        }
        
        return pulsar::interpreter::Token{"0", pulsar::interpreter::TokenType::INT};
    }

    if (operation == "help") {
        if (tokens.size() == 1) {
            printAllCommands();
        } else if (tokens.size() == 2) {
            printCommandHelp(tokens[1].value);
        } else {
            throw std::runtime_error("help: Too many arguments. Usage: help or help <command>");
        }
		return pulsar::interpreter::Token{"0", pulsar::interpreter::TokenType::INT};
    }

    if (operation == "version") {
		std::cout << pulsar::version() << std::endl;
		return pulsar::interpreter::Token{"0", pulsar::interpreter::TokenType::INT};
    }
    
    if ((getStandardLibraryFunctions()).count(operation)) {
		std::vector<pulsar::interpreter::Token> args(tokens.begin() + 1, tokens.end());
        auto resolvedArgs = resolveArguments(args);
        auto result = (getStandardLibraryFunctions())[operation](resolvedArgs);
        
        if (DEBUG) {
            std::cout << "PULSAR: Function '" << operation << "' returned: ";
			std::cout << "("
			          << (result.type == pulsar::interpreter::TokenType::INT      ? "INT"
			              : 
                              result.type == pulsar::interpreter::TokenType::STRING ? "STRING"
			                                                                      : "BOOLEAN")
			          << ") ";
			if (result.type == pulsar::interpreter::TokenType::STRING)
			{
                std::cout << "\"" << result.value << "\"";
            } else {
                std::cout << result.value;
            }
            std::cout << std::endl;
        }
        
        return result;
    }
    
    if (operation == "return") {
        if (tokens.size() < 2) {
            throw std::runtime_error("RETURN:0");
        }
        
        std::vector<Token> valueTokens(tokens.begin() + 1, tokens.end());
        auto resolvedArgs = resolveArguments(valueTokens);
        
        if (resolvedArgs.size() != 1) {
            throw std::runtime_error("return: Expected single return value");
        }
        
        throw std::runtime_error("RETURN:" + resolvedArgs[0].value);
    }
    
    if (operation == "while") {
        if (tokens.size() < 2) {
            throw std::runtime_error("while: Missing condition");
        }
        
        size_t conditionEnd = 1;
        size_t blockStart = tokens.size();
        
        for (size_t i = 1; i < tokens.size(); ++i) {
            if (tokens[i].type == TokenType::BLOCK_START) {
                conditionEnd = i;
                blockStart = i;
                break;
            }
        }
        
        if (blockStart == tokens.size()) {
            throw std::runtime_error("while: Missing opening '{' after condition");
        }
        
        size_t blockEnd = blockStart;
        int braceDepth = 0;
        for (size_t i = blockStart; i < tokens.size(); ++i) {
            if (tokens[i].type == TokenType::BLOCK_START) {
                braceDepth++;
            } else if (tokens[i].type == TokenType::BLOCK_END) {
                braceDepth--;
                if (braceDepth == 0) {
                    blockEnd = i;
                    break;
                }
            }
        }
        
        if (braceDepth != 0) {
            throw std::runtime_error("while: Missing closing '}' for while block");
        }
        
        std::vector<Token> conditionTokens(tokens.begin() + 1, tokens.begin() + conditionEnd);
        std::vector<Token> bodyTokens(tokens.begin() + blockStart + 1, tokens.begin() + blockEnd);
        
        while (true) {
            auto resolvedCondition = resolveArguments(conditionTokens);
            Token conditionResult = evaluateCondition(resolvedCondition);
            
            if (conditionResult.value != "true") {
                break;
            }
            
            executeBlock(bodyTokens);
        }
        
        return Token{"0", TokenType::INT};
    }
    
    if (functionTable.count(operation)) {
        std::vector<Token> args(tokens.begin() + 1, tokens.end());
        auto resolvedArgs = resolveArguments(args);
        return callFunction(operation, resolvedArgs);
    }


    throw std::runtime_error("Unknown operation '" + operation + "'. "
                           "Check spelling or read the documentation.");
}

int pulsar::interpreter::runCommand(const std::string &command)
{
    if (command.empty()) {
        return 0;
    }
    
    if (DEBUG) std::cout << "PULSAR: Running command from phasor: " << command << std::endl;

    try {
        auto commands = splitCommands(command);  
        for (const auto& cmd : commands) {
            auto tokens = tokenize(cmd);        
            auto lexedTokens = lexer(tokens);    
            parser(lexedTokens);                 
        }
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

int pulsar::interpreter::runCommands(const std::vector<std::string> &commands)
{
    for (const auto& cmd : commands) {
		int result = pulsar::interpreter::runCommand(cmd);
        if (result != 0) {
            return result;
        }
    }
    return 0;
}