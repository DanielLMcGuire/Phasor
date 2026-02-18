#pragma once
#include "../../../AST/AST.hpp"
#include <memory>
#include <vector>
/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/// @brief Parser
class Parser
{
  public:
	Parser(const std::vector<Token> &tokens);
	std::unique_ptr<AST::Program> parse();

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

	std::unique_ptr<AST::Statement>          declaration();
	std::unique_ptr<AST::Statement>          varDeclaration();
	std::unique_ptr<AST::Statement>          functionDeclaration();
	std::unique_ptr<AST::Statement>          statement();
	std::unique_ptr<AST::Statement>          printStatement();
	std::unique_ptr<AST::Statement>          ifStatement();
	std::unique_ptr<AST::Statement>          whileStatement();
	std::unique_ptr<AST::Statement>          forStatement();
	std::unique_ptr<AST::Statement>          switchStatement();
	std::unique_ptr<AST::Statement>          returnStatement();
	std::unique_ptr<AST::Statement>          unsafeStatement();
	std::unique_ptr<AST::BlockStmt>          block();
	std::unique_ptr<AST::Statement>          importStatement();
	std::unique_ptr<AST::Statement>          exportStatement();
	std::unique_ptr<AST::Statement>          expressionStatement();
	std::unique_ptr<AST::TypeNode>           parseType();
	std::unique_ptr<AST::Expression>         expression();
	std::unique_ptr<AST::Expression>         assignment();
	std::unique_ptr<AST::Expression>         logicalOr();
	std::unique_ptr<AST::Expression>         logicalAnd();
	std::unique_ptr<AST::Expression>         equality();
	std::unique_ptr<AST::Expression>         comparison();
	std::unique_ptr<AST::Expression>         term();
	std::unique_ptr<AST::Expression>         factor();
	std::unique_ptr<AST::Expression>         unary();
	std::unique_ptr<AST::Expression>         call();
	std::unique_ptr<AST::Expression>         finishCall(std::unique_ptr<AST::Expression> callee);
	std::unique_ptr<AST::Expression>         primary();
	std::unique_ptr<AST::StructDecl>         structDecl();
	std::unique_ptr<AST::StructInstanceExpr> structInstance();
	std::unique_ptr<AST::Expression>         fieldAccess(std::unique_ptr<AST::Expression> object);
};
} // namespace Phasor