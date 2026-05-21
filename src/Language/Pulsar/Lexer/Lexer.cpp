#include "Lexer.hpp"
#include <cctype>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <phsint.hpp>

namespace pulsar
{

Lexer::Lexer(std::string source) : source(std::move(source))
{
}

void Lexer::skipShebang()
{
	if (position == 0 && peek() == '#' && position + 1 < source.length() && source[position + 1] == '!')
	{
		while (!isAtEnd() && peek() != '\n')
		{
			advance();
		}
	}
}

std::vector<Phasor::Token> Lexer::tokenize()
{
	std::vector<Phasor::Token> tokens;
	skipShebang();
	while (!isAtEnd())
	{
		skipWhitespace();
		if (isAtEnd())
		{
			break;
		}
		tokens.push_back(scanToken());
	}
	tokens.push_back({Phasor::TokenType::EndOfFile, "", line, column});
	return tokens;
}

char Lexer::peek()
{
	if (isAtEnd())
	{
		return '\0';
	}
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
		if (std::isspace(static_cast<unsigned char>(c)) != 0)
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

Phasor::Token Lexer::scanToken()
{
	char c = peek();
	if (std::isalpha(static_cast<unsigned char>(c)) != 0)
	{
		return identifier();
	}
	if (std::isdigit(static_cast<unsigned char>(c)) != 0)
	{
		return number();
	}
	if (c == '"')
	{
		return string();
	}
	if (c == '`')
	{
		return complexString();
	}

	// Multi-character operators
	if (c == '+' && position + 1 < source.length() && source[position + 1] == '+')
	{
		advance();
		advance();
		return {Phasor::TokenType::Symbol, "++", line, column};
	}
	if (c == '-' && position + 1 < source.length() && source[position + 1] == '-')
	{
		advance();
		advance();
		return {Phasor::TokenType::Symbol, "--", line, column};
	}
	if (c == '=' && position + 1 < source.length() && source[position + 1] == '=')
	{
		advance();
		advance();
		return {Phasor::TokenType::Symbol, "==", line, column};
	}
	if (c == '!' && position + 1 < source.length() && source[position + 1] == '=')
	{
		advance();
		advance();
		return {Phasor::TokenType::Symbol, "!=", line, column};
	}
	if (c == '-' && position + 1 < source.length() && source[position + 1] == '>')
	{
		advance();
		advance();
		return {Phasor::TokenType::Symbol, "->", line, column};
	}
	if (c == '<' && position + 1 < source.length() && source[position + 1] == '=')
	{
		advance();
		advance();
		return {Phasor::TokenType::Symbol, "<=", line, column};
	}
	if (c == '>' && position + 1 < source.length() && source[position + 1] == '=')
	{
		advance();
		advance();
		return {Phasor::TokenType::Symbol, ">=", line, column};
	}
	if (c == '&' && position + 1 < source.length() && source[position + 1] == '&')
	{
		advance();
		advance();
		return {Phasor::TokenType::Symbol, "&&", line, column};
	}
	if (c == '|' && position + 1 < source.length() && source[position + 1] == '|')
	{
		advance();
		advance();
		return {Phasor::TokenType::Symbol, "||", line, column};
	}

	// Single-character symbols (parentheses, operators, punctuation, etc.)
	if (std::string("()+-*/%<>=!&|.{}:;,[]").find(c) != std::string::npos)
	{
		advance();
		return {Phasor::TokenType::Symbol, std::string(1, c), line, column};
	}

	advance();
	return {Phasor::TokenType::Unknown, std::string(1, c), line, column};
}

Phasor::Token Lexer::identifier()
{
	size_t start = position;
	while ((std::isalnum(static_cast<unsigned char>(peek())) != 0) || peek() == '_')
	{
		advance();
	}
	std::string text = source.substr(start, position - start);

	static const std::vector<std::string> keywords = {"let", "func", "print", "if", "else", "while"};

	for (const auto &kw : keywords)
	{
		if (text == kw)
		{
			return {Phasor::TokenType::Keyword, text, line, column};
		}
	}

	return {Phasor::TokenType::Identifier, text, line, column};
}

Phasor::Token Lexer::number()
{
	size_t start = position;
	while (std::isdigit(static_cast<unsigned char>(peek())) != 0)
	{
		advance();
	}
	if (peek() == '.' && position + 1 < source.length() &&
	    (std::isdigit(static_cast<unsigned char>(source[position + 1])) != 0))
	{
		advance();
		while (std::isdigit(static_cast<unsigned char>(peek())) != 0)
		{
			advance();
		}
	}
	return {Phasor::TokenType::Number, source.substr(start, position - start), line, column};
}

static int hexValue(char c)
{
	if (c >= '0' && c <= '9')
	{
		return c - '0';
	}
	if (c >= 'a' && c <= 'f')
	{
		return 10 + (c - 'a');
	}
	if (c >= 'A' && c <= 'F')
	{
		return 10 + (c - 'A');
	}
	return -1;
}

Phasor::Token Lexer::string()
{
	size_t             tokenLine = line;
	size_t             tokenColumn = column;
	std::ostringstream out;
	advance(); // Skip opening quote

	while (!isAtEnd())
	{
		char c = advance();

		if (c == '\n')
			return {Phasor::TokenType::Unknown, std::string(), tokenLine, tokenColumn};

		if (c == '\\')
		{
			if (isAtEnd())
				return {Phasor::TokenType::Unknown, std::string(), tokenLine, tokenColumn};

			char esc = advance();
			switch (esc)
			{
			case 'a':  out << '\a'; break;
			case 'b':  out << '\b'; break;
			case 'f':  out << '\f'; break;
			case 'n':  out << '\n'; break;
			case 'r':  out << '\r'; break;
			case 't':  out << '\t'; break;
			case 'v':  out << '\v'; break;
			case '\\': out << '\\'; break;
			case '\'': out << '\''; break;
			case '"':  out << '"';  break;

			case 'e':
			case 'E':
				out << '\x1b';
				break;

			case '0': case '1': case '2': case '3':
			case '4': case '5': case '6': case '7':
			{
				u32 val = static_cast<u32>(esc - '0');
				for (int i = 1; i < 3 && !isAtEnd(); ++i)
				{
					char d = peek();
					if (d < '0' || d > '7') break;
					advance();
					val = val * 8 + static_cast<u32>(d - '0');
				}
				if (val > 0xFF)
					return {Phasor::TokenType::Unknown, std::string(), tokenLine, tokenColumn};
				out << static_cast<char>(val);
				break;
			}

			case 'x':
			{
				if (isAtEnd() || hexValue(peek()) < 0)
					return {Phasor::TokenType::Unknown, std::string(), tokenLine, tokenColumn};
				int val = hexValue(advance());
				if (!isAtEnd() && hexValue(peek()) >= 0)
					val = (val << 4) | hexValue(advance());
				out << static_cast<char>(val);
				break;
			}

			case 'u':
			case 'U':
			{
				int      ndigits = (esc == 'u') ? 4 : 8;
				u32 cp      = 0;
				for (int i = 0; i < ndigits; ++i)
				{
					if (isAtEnd() || hexValue(peek()) < 0)
						return {Phasor::TokenType::Unknown, std::string(), tokenLine, tokenColumn};
					cp = (cp << 4) | static_cast<u32>(hexValue(advance()));
				}
				if (cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
					return {Phasor::TokenType::Unknown, std::string(), tokenLine, tokenColumn};
				if      (cp <= 0x7F)   { out << static_cast<char>(cp); }
				else if (cp <= 0x7FF)  { out << static_cast<char>(0xC0 | (cp >> 6))
				                            << static_cast<char>(0x80 | (cp & 0x3F)); }
				else if (cp <= 0xFFFF) { out << static_cast<char>(0xE0 | (cp >> 12))
				                            << static_cast<char>(0x80 | ((cp >> 6) & 0x3F))
				                            << static_cast<char>(0x80 | (cp & 0x3F)); }
				else                   { out << static_cast<char>(0xF0 | (cp >> 18))
				                            << static_cast<char>(0x80 | ((cp >> 12) & 0x3F))
				                            << static_cast<char>(0x80 | ((cp >> 6) & 0x3F))
				                            << static_cast<char>(0x80 | (cp & 0x3F)); }
				break;
			}

			default:
				out << esc;
				break;
			}
		}
		else if (c == '"')
		{
			return {Phasor::TokenType::String, out.str(), tokenLine, tokenColumn};
		}
		else
		{
			out << c;
		}
	}

	return {Phasor::TokenType::Unknown, std::string(), tokenLine, tokenColumn};
}

Phasor::Token Lexer::complexString()
{
	size_t             tokenLine = line;
	size_t             tokenColumn = column;
	std::ostringstream out;
	advance(); // Skip opening backtick

	// Not even attempting ${} syntax for now. Just read as a raw string.

	while (!isAtEnd())
	{
		char c = advance();

		if (c == '`')
		{
			// Closing backtick
			return {Phasor::TokenType::String, out.str(), tokenLine, tokenColumn};
		}

		out << c;
	}

	// If we get here, string was unterminated
	return {Phasor::TokenType::Unknown, std::string(), tokenLine, tokenColumn};
}
} // namespace pulsar