#pragma once
#include "../../../AST/AST.hpp"
#include <string>
#include <vector>
#include <iostream>

namespace Phasor
{
/// @brief Lexer
class Lexer
{
  public:
	Lexer(const std::string &source);
	std::vector<Token> tokenize();

  private:
	std::string source;
	size_t      position = 0;
	size_t      line = 1;
	size_t      column = 1;

	char  peek();
	char  advance();
	bool  isAtEnd();
	void  skipWhitespace();
	void  skipShebang();
	Token scanToken();
	Token identifier();
	Token number();
	Token string();
	Token complexString();
};
} // namespace Phasor
