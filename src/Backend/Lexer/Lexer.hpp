#pragma once
#include <string>
#include <vector>
#include <iostream>

enum class TokenType
{
	Identifier,
	Number,
	String,
	Keyword,
	Symbol,
	EndOfFile,
	Unknown
};

struct Token
{
	TokenType   type;
	std::string lexeme;
	int         line;
	int         column;
};

class Lexer
{
  public:
	Lexer(const std::string &source);
	std::vector<Token> tokenize();

  private:
	std::string source;
	int         position = 0;
	int         line = 1;
	int         column = 1;

	char  peek();
	char  advance();
	bool  isAtEnd();
	void  skipWhitespace();
	Token scanToken();
	Token identifier();
	Token number();
	Token string();
};
