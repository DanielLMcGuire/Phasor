#include "Parser.hpp"
#include <iostream>

namespace pulsar
{

Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens)
{
}

std::unique_ptr<Program> Parser::parse()
{
	auto program = std::make_unique<Program>();
	while (!isAtEnd())
	{
		program->statements.push_back(declaration());
	}
	return program;
}

std::unique_ptr<Statement> Parser::declaration()
{
	if (check(TokenType::Keyword) && peek().lexeme == "func")
	{
		advance();
		return functionDeclaration();
	}
	if (check(TokenType::Keyword) && peek().lexeme == "let")
	{
		advance();
		return varDeclaration();
	}
	return statement();
}

std::unique_ptr<Statement> Parser::functionDeclaration()
{
	Token name = consume(TokenType::Identifier, "Expect function name.");
	consume(TokenType::Symbol, "(", "Expect '(' after function name.");
	std::vector<FunctionDecl::Param> params;
	if (!check(TokenType::Symbol) || peek().lexeme != ")")
	{
		do
		{
			Token paramName = consume(TokenType::Identifier, "Expect parameter name.");
			consume(TokenType::Symbol, ":", "Expect ':' after parameter name.");
			auto type = parseType();
			params.push_back({paramName.lexeme, std::move(type)});
		} while (match(TokenType::Symbol, ","));
	}
	consume(TokenType::Symbol, ")", "Expect ')' after parameters.");

	std::unique_ptr<TypeNode> returnType = nullptr;
	if (match(TokenType::Symbol, "->"))
	{
		returnType = parseType();
	}

	consume(TokenType::Symbol, "{", "Expect '{' before function body.");

	// Track function context for better error messages
	std::string previousFunction = currentFunction;
	currentFunction = name.lexeme;

	auto body = block();

	// Restore previous function context
	currentFunction = previousFunction;

	return std::make_unique<FunctionDecl>(name.lexeme, std::move(params), std::move(returnType), std::move(body));
}

std::unique_ptr<TypeNode> Parser::parseType()
{
	bool isPointer = false;
	if (match(TokenType::Symbol, "*"))
	{
		isPointer = true;
	}
	Token typeName = consume(TokenType::Identifier, "Expect type name.");

	std::vector<int> dims;
	while (match(TokenType::Symbol, "["))
	{
		Token size = consume(TokenType::Number, "Expect array size in type declaration.");
		dims.push_back(std::stoi(size.lexeme));
		consume(TokenType::Symbol, "]", "Expect ']' after array size.");
	}
	return std::make_unique<TypeNode>(typeName.lexeme, isPointer, dims);
}

std::unique_ptr<Statement> Parser::varDeclaration()
{
	Token name = consume(TokenType::Identifier, "Expect variable name.");
	consume(TokenType::Symbol, "=", "Expect '=' after variable name.");
	std::unique_ptr<Expression> initializer = nullptr;
	initializer = expression();
	return std::make_unique<VarDecl>(name.lexeme, std::move(initializer));
}

std::unique_ptr<Statement> Parser::statement()
{
	if (check(TokenType::Keyword) && peek().lexeme == "print")
	{
		advance();
		return printStatement();
	}
	if (check(TokenType::Keyword) && peek().lexeme == "if")
	{
		advance();
		return ifStatement();
	}
	if (check(TokenType::Keyword) && peek().lexeme == "while")
	{
		advance();
		return whileStatement();
	}
	if (check(TokenType::Keyword) && peek().lexeme == "return")
	{
		advance();
		return returnStatement();
	}
	if (check(TokenType::Keyword) && peek().lexeme == "break")
	{
		advance();
		return std::make_unique<BreakStmt>();
	}
	if (check(TokenType::Keyword) && peek().lexeme == "continue")
	{
		advance();
		return std::make_unique<ContinueStmt>();
	}
	if (check(TokenType::Symbol) && peek().lexeme == "{")
	{
		advance();
		auto blk = block();
		return blk;
	}
	return expressionStatement();
}

std::unique_ptr<Statement> Parser::ifStatement()
{
	consume(TokenType::Symbol, "(", "Expect '(' after 'if'.");
	auto condition = expression();
	consume(TokenType::Symbol, ")", "Expect ')' after if condition.");
	auto                       thenBranch = statement();
	std::unique_ptr<Statement> elseBranch = nullptr;
	if (match(TokenType::Keyword, "else"))
	{
		elseBranch = statement();
	}
	return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Statement> Parser::whileStatement()
{
	consume(TokenType::Symbol, "(", "Expect '(' after 'while'.");
	auto condition = expression();
	consume(TokenType::Symbol, ")", "Expect ')' after while condition.");
	auto body = statement();
	return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Statement> Parser::forStatement()
{
	consume(TokenType::Symbol, "(", "Expect '(' after 'for'.");

	std::unique_ptr<Statement> initializer = nullptr;
	if (check(TokenType::Keyword) && peek().lexeme == "let")
	{
		advance();
		initializer = varDeclaration();
	}
	else
	{
		initializer = expressionStatement();
	}

	std::unique_ptr<Expression> condition = nullptr;
	condition = expression();

	std::unique_ptr<Expression> increment = nullptr;
	if (!check(TokenType::Symbol) || peek().lexeme != ")")
	{
		increment = expression();
	}
	consume(TokenType::Symbol, ")", "Expect ')' after for clauses.");

	auto body = statement();
	return std::make_unique<ForStmt>(std::move(initializer), std::move(condition), std::move(increment),
	                                 std::move(body));
}

std::unique_ptr<Statement> Parser::switchStatement()
{
	consume(TokenType::Symbol, "(", "Expect '(' after 'switch'.");
	auto expr = expression();
	consume(TokenType::Symbol, ")", "Expect ')' after switch expression.");
	consume(TokenType::Symbol, "{", "Expect '{' after switch.");

	std::vector<CaseClause>                 cases;
	std::vector<std::unique_ptr<Statement>> defaultStmts;

	while (!check(TokenType::Symbol) || peek().lexeme != "}")
	{
		if (isAtEnd())
			throw std::runtime_error("Unterminated switch statement.");

		if (check(TokenType::Keyword) && peek().lexeme == "case")
		{
			advance();
			auto caseValue = expression();
			consume(TokenType::Symbol, ":", "Expect ':' after case value.");

			std::vector<std::unique_ptr<Statement>> stmts;
			while ((!check(TokenType::Keyword) || (peek().lexeme != "case" && peek().lexeme != "default")) &&
			       (!check(TokenType::Symbol) || peek().lexeme != "}"))
			{
				if (isAtEnd())
					throw std::runtime_error("Unterminated case clause.");
				stmts.push_back(declaration());
			}
			cases.emplace_back(std::move(caseValue), std::move(stmts));
		}
		else if (check(TokenType::Keyword) && peek().lexeme == "default")
		{
			advance();
			consume(TokenType::Symbol, ":", "Expect ':' after default.");

			while ((!check(TokenType::Keyword) || (peek().lexeme != "case" && peek().lexeme != "default")) &&
			       (!check(TokenType::Symbol) || peek().lexeme != "}"))
			{
				if (isAtEnd())
					throw std::runtime_error("Unterminated default clause.");
				defaultStmts.push_back(declaration());
			}
		}
		else
		{
			throw std::runtime_error("Expected 'case' or 'default' in switch statement.");
		}
	}

	consume(TokenType::Symbol, "}", "Expect '}' after switch body.");
	return std::make_unique<SwitchStmt>(std::move(expr), std::move(cases), std::move(defaultStmts));
}

std::unique_ptr<Statement> Parser::returnStatement()
{
	std::unique_ptr<Expression> value = nullptr;
	value = expression();
	return std::make_unique<ReturnStmt>(std::move(value));
}

std::unique_ptr<BlockStmt> Parser::block()
{
	std::vector<std::unique_ptr<Statement>> statements;
	while (!check(TokenType::Symbol) || peek().lexeme != "}")
	{
		if (isAtEnd())
			throw std::runtime_error("Unterminated block.");
		statements.push_back(declaration());
	}
	consume(TokenType::Symbol, "}", "Expect '}' after block.");
	return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Statement> Parser::printStatement()
{
	auto expr = expression();
	return std::make_unique<PrintStmt>(std::move(expr));
}

std::unique_ptr<Statement> Parser::expressionStatement()
{
	auto expr = expression();
	return std::make_unique<ExpressionStmt>(std::move(expr));
}

std::unique_ptr<Expression> Parser::expression()
{
	return assignment();
}

std::unique_ptr<Expression> Parser::assignment()
{
	auto expr = logicalOr();

	if (match(TokenType::Symbol, "="))
	{
		auto value = assignment();
		return std::make_unique<AssignmentExpr>(std::move(expr), std::move(value));
	}

	return expr;
}

std::unique_ptr<Expression> Parser::logicalOr()
{
	auto expr = logicalAnd();

	while (check(TokenType::Symbol) && peek().lexeme == "||")
	{
		advance();
		auto right = logicalAnd();
		expr = std::make_unique<BinaryExpr>(std::move(expr), BinaryOp::Or, std::move(right));
	}

	return expr;
}

std::unique_ptr<Expression> Parser::logicalAnd()
{
	auto expr = equality();

	while (check(TokenType::Symbol) && peek().lexeme == "&&")
	{
		advance();
		auto right = equality();
		expr = std::make_unique<BinaryExpr>(std::move(expr), BinaryOp::And, std::move(right));
	}

	return expr;
}

std::unique_ptr<Expression> Parser::equality()
{
	auto expr = comparison();

	while (check(TokenType::Symbol) && (peek().lexeme == "==" || peek().lexeme == "!="))
	{
		Token    op = advance();
		auto     right = comparison();
		BinaryOp binOp = (op.lexeme == "==") ? BinaryOp::Equal : BinaryOp::NotEqual;
		expr = std::make_unique<BinaryExpr>(std::move(expr), binOp, std::move(right));
	}

	return expr;
}

std::unique_ptr<Expression> Parser::comparison()
{
	auto expr = term();

	while (check(TokenType::Symbol) &&
	       (peek().lexeme == "<" || peek().lexeme == ">" || peek().lexeme == "<=" || peek().lexeme == ">="))
	{
		Token    op = advance();
		auto     right = term();
		BinaryOp binOp;
		if (op.lexeme == "<")
			binOp = BinaryOp::LessThan;
		else if (op.lexeme == ">")
			binOp = BinaryOp::GreaterThan;
		else if (op.lexeme == "<=")
			binOp = BinaryOp::LessEqual;
		else
			binOp = BinaryOp::GreaterEqual;

		expr = std::make_unique<BinaryExpr>(std::move(expr), binOp, std::move(right));
	}

	return expr;
}

std::unique_ptr<Expression> Parser::term()
{
	auto expr = factor();

	while (check(TokenType::Symbol) && (peek().lexeme == "+" || peek().lexeme == "-"))
	{
		Token    op = advance();
		auto     right = factor();
		BinaryOp binOp = (op.lexeme == "+") ? BinaryOp::Add : BinaryOp::Subtract;
		expr = std::make_unique<BinaryExpr>(std::move(expr), binOp, std::move(right));
	}

	return expr;
}

std::unique_ptr<Expression> Parser::factor()
{
	auto expr = unary();

	while (check(TokenType::Symbol) && (peek().lexeme == "*" || peek().lexeme == "/" || peek().lexeme == "%"))
	{
		Token    op = advance();
		auto     right = unary();
		BinaryOp binOp;
		if (op.lexeme == "*")
			binOp = BinaryOp::Multiply;
		else if (op.lexeme == "/")
			binOp = BinaryOp::Divide;
		else
			binOp = BinaryOp::Modulo;

		expr = std::make_unique<BinaryExpr>(std::move(expr), binOp, std::move(right));
	}

	return expr;
}

std::unique_ptr<Expression> Parser::unary()
{
	if (check(TokenType::Symbol) && (peek().lexeme == "!" || peek().lexeme == "-"))
	{
		Token   op = advance();
		auto    right = unary();
		UnaryOp uOp = (op.lexeme == "!") ? UnaryOp::Not : UnaryOp::Negate;
		return std::make_unique<UnaryExpr>(uOp, std::move(right));
	}
	if (check(TokenType::Symbol) && peek().lexeme == "&")
	{
		advance();
		auto right = unary();
		return std::make_unique<UnaryExpr>(UnaryOp::AddressOf, std::move(right));
	}
	if (check(TokenType::Symbol) && peek().lexeme == "*")
	{
		advance();
		auto right = unary();
		return std::make_unique<UnaryExpr>(UnaryOp::Dereference, std::move(right));
	}
	return call();
}

std::unique_ptr<Expression> Parser::call()
{
	auto expr = primary();

	while (true)
	{
		if (match(TokenType::Symbol, "("))
		{
			expr = finishCall(std::move(expr));
		}
		else if (match(TokenType::Symbol, "++"))
		{
			expr = std::make_unique<PostfixExpr>(PostfixOp::Increment, std::move(expr));
		}
		else if (match(TokenType::Symbol, "--"))
		{
			expr = std::make_unique<PostfixExpr>(PostfixOp::Decrement, std::move(expr));
		}
		else
		{
			break;
		}
	}

	return expr;
}

std::unique_ptr<Expression> Parser::finishCall(std::unique_ptr<Expression> callee)
{
	std::vector<std::unique_ptr<Expression>> arguments;
	if (!check(TokenType::Symbol) || peek().lexeme != ")")
	{
		do
		{
			arguments.push_back(expression());
		} while (match(TokenType::Symbol, ","));
	}
	consume(TokenType::Symbol, ")", "Expect ')' after arguments.");

	// For now, we only support direct function calls by name, or calls on field access which are
	// rewritten to pass the object as the first argument.
	if (auto ident = dynamic_cast<IdentifierExpr *>(callee.get()))
	{
		return std::make_unique<CallExpr>(ident->name, std::move(arguments));
	}
	else if (auto field = dynamic_cast<FieldAccessExpr *>(callee.get()))
	{
		// Transform obj.method(args) -> method(obj, args)
		std::string methodName = field->fieldName;
		arguments.insert(arguments.begin(), std::move(field->object));
		return std::make_unique<CallExpr>(methodName, std::move(arguments));
	}

	throw std::runtime_error("Can only call named functions.");
}

std::unique_ptr<Expression> Parser::primary()
{
	if (match(TokenType::Number))
	{
		return std::make_unique<NumberExpr>(previous().lexeme);
	}
	if (match(TokenType::String))
	{
		return std::make_unique<StringExpr>(previous().lexeme);
	}
	if (check(TokenType::Identifier))
	{
		Token identTok = peek();
		advance();
		return std::make_unique<IdentifierExpr>(identTok.lexeme);
	}
	if (match(TokenType::Keyword, "true"))
	{
		return std::make_unique<BooleanExpr>(true);
	}
	if (match(TokenType::Keyword, "false"))
	{
		return std::make_unique<BooleanExpr>(false);
	}
	if (match(TokenType::Keyword, "null"))
	{
		return std::make_unique<NullExpr>();
	}
	if (match(TokenType::Symbol, "("))
	{
		auto expr = expression();
		consume(TokenType::Symbol, ")", "Expect ')' after expression.");
		return expr;
	}
	std::cerr << "Error: Expect expression at '" << peek().lexeme << "'";
	std::cerr << " (line " << peek().line << ", column " << peek().column << ")\n";
	throw std::runtime_error("Expect expression.");
}

Token Parser::peek()
{
	return tokens[current];
}

Token Parser::peekNext()
{
	if (current + 1 >= static_cast<int>(tokens.size()))
	{
		return tokens[current];
	}
	return tokens[current + 1];
}

Token Parser::previous()
{
	return tokens[current - 1];
}

Token Parser::advance()
{
	if (!isAtEnd())
		current++;
	return previous();
}

bool Parser::isAtEnd()
{
	return peek().type == TokenType::EndOfFile;
}

bool Parser::check(TokenType type)
{
	if (isAtEnd())
		return false;
	return peek().type == type;
}

bool Parser::match(TokenType type)
{
	if (check(type))
	{
		advance();
		return true;
	}
	return false;
}

Token Parser::consume(TokenType type, std::string message)
{
	if (check(type))
		return advance();

	std::cerr << "Error: " << message << " at '" << peek().lexeme << "'";
	std::cerr << " (line " << peek().line << ", column " << peek().column << ")";
	if (!currentFunction.empty())
		std::cerr << " [in function '" << currentFunction << "']";
	std::cerr << "\n";

	throw std::runtime_error(message);
}

bool Parser::match(TokenType type, std::string lexeme)
{
	if (check(type) && peek().lexeme == lexeme)
	{
		advance();
		return true;
	}
	return false;
}

Token Parser::consume(TokenType type, std::string lexeme, std::string message)
{
	if (check(type) && peek().lexeme == lexeme)
	{
		return advance();
	}

	std::cerr << "Error: " << message << " at '" << peek().lexeme << "'";
	std::cerr << " (line " << peek().line << ", column " << peek().column << ")";
	if (!currentFunction.empty())
		std::cerr << " [in function '" << currentFunction << "']";
	std::cerr << "\n";

	throw std::runtime_error(message);
}

Token Parser::expect(TokenType type, const std::string &message)
{
	if (check(type))
	{
		return advance();
	}
	throw std::runtime_error(message);
}
} // namespace pulsar