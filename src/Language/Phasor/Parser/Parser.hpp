#pragma once
#include "../../../AST/AST.hpp"
#include <memory>
#include <optional>
#include <vector>
#include <filesystem>
/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/// @brief Parser
class Parser
{
  public:
	Parser(const std::vector<Token> &tokens);
	Parser(const std::vector<Token> &tokens, const std::filesystem::path &sourcePath);

	inline void setSourcePath(const std::filesystem::path &path)
	{
		sourcePath = path;
	}

	std::unique_ptr<AST::Program> parse();
	
	struct Error
	{
		std::string message;
		size_t      line;
		size_t      column;
	};
	std::optional<Error> getError() const
	{
		return lastError;
	}
  private:
	std::vector<Token> tokens;
	int                current = 0;
	std::string        currentFunction = "";
	std::optional<Error> lastError;
	std::filesystem::path sourcePath;

	Token peek();
	Token previous();
	Token advance();
	bool  isAtEnd();
	bool  check(Phasor::TokenType type);
	Token peekNext();
	bool  match(Phasor::TokenType type);
	bool  match(Phasor::TokenType type, std::string lexeme);
	Token consume(Phasor::TokenType type, std::string message);
	Token consume(Phasor::TokenType type, std::string lexeme, std::string message);
	Token expect(Phasor::TokenType type, const std::string &message);

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