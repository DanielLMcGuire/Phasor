#include "Lexer.hpp"
#include <cctype>

Lexer::Lexer(const std::string &source) : source(source)
{
}

std::vector<Token> Lexer::tokenize()
{
	std::vector<Token> tokens;
	while (!isAtEnd())
	{
		skipWhitespace();
		if (isAtEnd())
			break;
		tokens.push_back(scanToken());
	}
	tokens.push_back({TokenType::EndOfFile, "", line, column});
	return tokens;
}

char Lexer::peek()
{
	if (isAtEnd())
		return '\0';
	return source[position];
}

char Lexer::advance()
{
	char c = source[position++];
	column++;
	if (c == '\n')
	{
		line++;
		column = 1;
	}
	return c;
}

bool Lexer::isAtEnd()
{
	return position >= source.length();
}

void Lexer::skipWhitespace()
{
	while (!isAtEnd())
	{
		char c = peek();
		if (std::isspace(c))
		{
			advance();
		}
		else if (c == '/' && position + 1 < source.length() && source[position + 1] == '/')
		{
			// Skip single-line comment
			while (!isAtEnd() && peek() != '\n')
			{
				advance();
			}
		}
		else
		{
			break;
		}
	}
}

Token Lexer::scanToken()
{
    char c = peek();
    if (std::isalpha(c))
        return identifier();
    if (std::isdigit(c))
        return number();
    if (c == '"')
        return string();

    // Multi-character operators
    if (c == '+' && position + 1 < source.length() && source[position + 1] == '+')
    {
        advance();
        advance();
        return {TokenType::Symbol, "++", line, column};
    }
    if (c == '-' && position + 1 < source.length() && source[position + 1] == '-')
    {
        advance();
        advance();
        return {TokenType::Symbol, "--", line, column};
    }
    if (c == '=' && position + 1 < source.length() && source[position + 1] == '=')
    {
        advance();
        advance();
        return {TokenType::Symbol, "==", line, column};
    }
    if (c == '!' && position + 1 < source.length() && source[position + 1] == '=')
    {
        advance();
        advance();
        return {TokenType::Symbol, "!=", line, column};
    }
    if (c == '-' && position + 1 < source.length() && source[position + 1] == '>')
    {
        advance();
        advance();
        return {TokenType::Symbol, "->", line, column};
    }
    if (c == '<' && position + 1 < source.length() && source[position + 1] == '=')
    {
        advance();
        advance();
        return {TokenType::Symbol, "<=", line, column};
    }
    if (c == '>' && position + 1 < source.length() && source[position + 1] == '=')
    {
        advance();
        advance();
        return {TokenType::Symbol, ">=", line, column};
    }
    if (c == '&' && position + 1 < source.length() && source[position + 1] == '&')
    {
        advance();
        advance();
        return {TokenType::Symbol, "&&", line, column};
    }
    if (c == '|' && position + 1 < source.length() && source[position + 1] == '|')
    {
        advance();
        advance();
        return {TokenType::Symbol, "||", line, column};
    }

    // Single-character symbols (parentheses, operators, punctuation, etc.)
    if (std::string("()+-*/%<>=!&|.{}:;,").find(c) != std::string::npos)
    {
        advance();
        return {TokenType::Symbol, std::string(1, c), line, column};
    }

    advance();
    return {TokenType::Unknown, std::string(1, c), line, column};
}

Token Lexer::identifier()
{
    int start = position;
    while (std::isalnum(peek()) || peek() == '_')
        advance();
    std::string text = source.substr(start, position - start);

    // Check for keywords
    static const std::vector<std::string> keywords = {
        "var",   "const", "fn",   "class",    "if",       "else",   "while", "for",   "return",
        "true",  "false", "null", "import",   "export",   "async",  "await", "throw", "try",
        "catch", "match", "enum", "template", "operator", "unsafe", "spawn", "print", "struct",
        "break", "continue", "switch", "case", "default"
    };

    for (const auto &kw : keywords)
    {
        if (text == kw)
        {
            return {TokenType::Keyword, text, line, column};
        }
    }

    return {TokenType::Identifier, text, line, column};
}

Token Lexer::number()
{
	int start = position;
	while (std::isdigit(peek()))
		advance();
	if (peek() == '.' && std::isdigit(source[position + 1]))
	{
		advance();
		while (std::isdigit(peek()))
			advance();
	}
	return {TokenType::Number, source.substr(start, position - start), line, column};
}

Token Lexer::string()
{
	int start = position;
	advance(); // Skip opening quote
	while (peek() != '"' && !isAtEnd())
	{
		advance();
	}
	if (isAtEnd())
	{
		// Error: Unterminated string
		return {TokenType::Unknown, source.substr(start, position - start), line, column};
	}
	advance(); // Skip closing quote
	return {TokenType::String, source.substr(start + 1, position - start - 2), line, column};
}
