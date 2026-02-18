#pragma once
#include "../../../AST/AST.hpp"
#include <memory>
#include <vector>
/// @brief The Pulsar Scripting Language
namespace pulsar
{

using namespace Phasor::AST;
using namespace Phasor;

/// @brief Parser
class Parser
{
  public:
	Parser(const std::vector<Token> &tokens);
	std::unique_ptr<Program> parse();

  private:
	std::vector<Token> tokens;
	int                current = 0;
	std::string        currentFunction = "";

	Token peek();
	Token previous();
	Token advance();
	bool  isAtEnd();
	bool  check(TokenType type);
	Token peekNext();
	bool  match(TokenType type);
	bool  match(TokenType type, std::string lexeme);
	Token consume(TokenType type, std::string message);
	Token consume(TokenType type, std::string lexeme, std::string message);
	Token expect(TokenType type, const std::string &message);

	std::unique_ptr<Statement>  declaration();
	std::unique_ptr<Statement>  varDeclaration();
	std::unique_ptr<Statement>  functionDeclaration();
	std::unique_ptr<Statement>  statement();
	std::unique_ptr<Statement>  printStatement();
	std::unique_ptr<Statement>  ifStatement();
	std::unique_ptr<Statement>  whileStatement();
	std::unique_ptr<Statement>  forStatement();
	std::unique_ptr<Statement>  switchStatement();
	std::unique_ptr<Statement>  returnStatement();
	std::unique_ptr<BlockStmt>  block();
	std::unique_ptr<Statement>  expressionStatement();
	std::unique_ptr<TypeNode>   parseType();
	std::unique_ptr<Expression> expression();
	std::unique_ptr<Expression> assignment();
	std::unique_ptr<Expression> logicalOr();
	std::unique_ptr<Expression> logicalAnd();
	std::unique_ptr<Expression> equality();
	std::unique_ptr<Expression> comparison();
	std::unique_ptr<Expression> term();
	std::unique_ptr<Expression> factor();
	std::unique_ptr<Expression> unary();
	std::unique_ptr<Expression> call();
	std::unique_ptr<Expression> finishCall(std::unique_ptr<Expression> callee);
	std::unique_ptr<Expression> primary();
};
} // namespace pulsar