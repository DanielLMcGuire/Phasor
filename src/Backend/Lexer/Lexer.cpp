#include "Lexer.hpp"
#include <cctype>
#include <sstream>
#include <stdexcept>

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
		if (std::isspace(static_cast<unsigned char>(c)))
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
    if (std::isalpha(static_cast<unsigned char>(c)))
        return identifier();
    if (std::isdigit(static_cast<unsigned char>(c)))
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
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_')
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
	while (std::isdigit(static_cast<unsigned char>(peek())))
		advance();
	if (peek() == '.' && position + 1 < source.length() && std::isdigit(static_cast<unsigned char>(source[position + 1])))
	{
		advance();
		while (std::isdigit(static_cast<unsigned char>(peek())))
			advance();
	}
	return {TokenType::Number, source.substr(start, position - start), line, column};
}

static int hexValue(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
	if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
	return -1;
}

Token Lexer::string()
{
	int tokenLine = line;
	int tokenColumn = column;
	std::ostringstream out;
	advance(); // Skip opening quote

	while (!isAtEnd())
	{
		char c = advance();

		// Raw newline inside a string is treated as unterminated/error.
		if (c == '\n')
		{
			// Unterminated string literal
			return {TokenType::Unknown, std::string(), tokenLine, tokenColumn};
		}

		if (c == '\\')
		{
			if (isAtEnd())
			{
				// Unterminated escape at end of file
				return {TokenType::Unknown, std::string(), tokenLine, tokenColumn};
			}
			char esc = advance();
			switch (esc)
			{
				case 'n': out << '\n'; break;
				case 't': out << '\t'; break;
				case 'r': out << '\r'; break;
				case '\\': out << '\\'; break;
				case '"': out << '"'; break;
				case '\'': out << '\''; break;
				case '0': out << '\0'; break;
				case 'b': out << '\b'; break;
				case 'f': out << '\f'; break;
				case 'v': out << '\v'; break;
				case 'x': {
					// Hex escape sequence: \xHH
					if (isAtEnd()) return {TokenType::Unknown, std::string(), tokenLine, tokenColumn};
					char h1 = advance();
					if (isAtEnd()) return {TokenType::Unknown, std::string(), tokenLine, tokenColumn};
					char h2 = advance();
					int v1 = hexValue(h1), v2 = hexValue(h2);
					if (v1 < 0 || v2 < 0)
						return {TokenType::Unknown, std::string(), tokenLine, tokenColumn};
					char value = static_cast<char>((v1 << 4) | v2);
					out << value;
					break;
				}
				default:
					// Unknown escape: be permissive and append the escaped character as-is.
					out << esc;
					break;
			}
		}
		else if (c == '"')
		{
			// Closing quote
			return {TokenType::String, out.str(), tokenLine, tokenColumn};
		}
		else
		{
			out << c;
		}
	}

	// If we get here, string was unterminated
	return {TokenType::Unknown, std::string(), tokenLine, tokenColumn};
}
