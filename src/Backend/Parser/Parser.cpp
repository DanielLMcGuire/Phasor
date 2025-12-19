#include "Parser.hpp"
#include <iostream>

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
	if (check(TokenType::Keyword) && peek().lexeme == "fn")
	{
		advance();
		return functionDeclaration();
	}
	if (check(TokenType::Keyword) && peek().lexeme == "var")
	{
		advance();
		return varDeclaration();
	}
	if (check(TokenType::Keyword) && peek().lexeme == "import")
	{
		advance();
		return importStatement();
	}
	if (check(TokenType::Keyword) && peek().lexeme == "export")
	{
		advance();
		return exportStatement();
	}
	if (check(TokenType::Keyword) && peek().lexeme == "struct")
	{
		return structDecl();
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
	// Handle generics later if needed, for now just basic types
	return std::make_unique<TypeNode>(typeName.lexeme, isPointer);
}

std::unique_ptr<Statement> Parser::varDeclaration()
{
	Token                       name = consume(TokenType::Identifier, "Expect variable name.");
	std::unique_ptr<Expression> initializer = nullptr;
	if (match(TokenType::Symbol, "="))
	{
		initializer = expression();
	}
	consume(TokenType::Symbol, ";", "Expect ';' after variable declaration.");
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
	if (check(TokenType::Keyword) && peek().lexeme == "for")
	{
		advance();
		return forStatement();
	}
	if (check(TokenType::Keyword) && peek().lexeme == "switch")
	{
		advance();
		return switchStatement();
	}
	if (check(TokenType::Keyword) && peek().lexeme == "return")
	{
		advance();
		return returnStatement();
	}
	if (check(TokenType::Keyword) && peek().lexeme == "break")
	{
		advance();
		consume(TokenType::Symbol, ";", "Expect ';' after 'break'.");
		return std::make_unique<BreakStmt>();
	}
	if (check(TokenType::Keyword) && peek().lexeme == "continue")
	{
		advance();
		consume(TokenType::Symbol, ";", "Expect ';' after 'continue'.");
		return std::make_unique<ContinueStmt>();
	}
	if (check(TokenType::Keyword) && peek().lexeme == "unsafe")
	{
		advance();
		return unsafeStatement();
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
	if (!check(TokenType::Symbol) || peek().lexeme != ";")
	{
		if (check(TokenType::Keyword) && peek().lexeme == "var")
		{
			advance();
			initializer = varDeclaration();
		}
		else
		{
			initializer = expressionStatement();
		}
	}
	else
	{
		consume(TokenType::Symbol, ";", "Expect ';'.");
	}
	
	std::unique_ptr<Expression> condition = nullptr;
	if (!check(TokenType::Symbol) || peek().lexeme != ";")
	{
		condition = expression();
	}
	consume(TokenType::Symbol, ";", "Expect ';' after loop condition.");
	
	std::unique_ptr<Expression> increment = nullptr;
	if (!check(TokenType::Symbol) || peek().lexeme != ")")
	{
		increment = expression();
	}
	consume(TokenType::Symbol, ")", "Expect ')' after for clauses.");
	
	auto body = statement();
	return std::make_unique<ForStmt>(std::move(initializer), std::move(condition),
	                                  std::move(increment), std::move(body));
}

std::unique_ptr<Statement> Parser::switchStatement()
{
	consume(TokenType::Symbol, "(", "Expect '(' after 'switch'.");
	auto expr = expression();
	consume(TokenType::Symbol, ")", "Expect ')' after switch expression.");
	consume(TokenType::Symbol, "{", "Expect '{' after switch.");

	std::vector<CaseClause> cases;
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
	if (!check(TokenType::Symbol) || peek().lexeme != ";")
	{
		value = expression();
	}
	consume(TokenType::Symbol, ";", "Expect ';' after return value.");
	return std::make_unique<ReturnStmt>(std::move(value));
}

std::unique_ptr<Statement> Parser::unsafeStatement()
{
	consume(TokenType::Symbol, "{", "Expect '{' after 'unsafe'.");
	return std::make_unique<UnsafeBlockStmt>(block());
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
	consume(TokenType::Symbol, ";", "Expect ';' after print statement.");
	return std::make_unique<PrintStmt>(std::move(expr));
}

std::unique_ptr<Statement> Parser::importStatement()
{
	Token path = consume(TokenType::String, "Expect string after 'import'.");
	consume(TokenType::Symbol, ";", "Expect ';' after import statement.");
	return std::make_unique<ImportStmt>(path.lexeme);
}

std::unique_ptr<Statement> Parser::exportStatement()
{
	return std::make_unique<ExportStmt>(declaration());
}

std::unique_ptr<Statement> Parser::expressionStatement()
{
	auto expr = expression();
	consume(TokenType::Symbol, ";", "Expect ';' after expression.");
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
		auto value = assignment(); // Right-associative
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
		else if (match(TokenType::Symbol, "."))
		{
			expr = fieldAccess(std::move(expr));
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
		// Look ahead for struct instance syntax: Name{ ... }
		Token identTok = peek();
		if (!isAtEnd() && peekNext().type == TokenType::Symbol && peekNext().lexeme == "{")
		{
			return structInstance();
		}
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

// Struct declaration
// struct Point { x: int, y: int }
std::unique_ptr<StructDecl> Parser::structDecl()
{
	// 'struct' keyword
	consume(TokenType::Keyword, "struct", "Expected 'struct'");
	Token nameTok = consume(TokenType::Identifier, "Expected struct name");
	consume(TokenType::Symbol, "{", "Expected '{' in struct declaration");

	std::vector<StructField> fields;
	while (!check(TokenType::Symbol) || peek().lexeme != "}")
	{
		if (isAtEnd())
			throw std::runtime_error("Unterminated struct declaration.");

		Token fieldNameTok = consume(TokenType::Identifier, "Expected field name");
		consume(TokenType::Symbol, ":", "Expected ':' after field name");
		auto type = parseType();
		fields.emplace_back(fieldNameTok.lexeme, std::move(type));

		if (!match(TokenType::Symbol, ","))
		{
			break;
		}
	}

	consume(TokenType::Symbol, "}", "Expected '}' after struct fields");
	return std::make_unique<StructDecl>(nameTok.lexeme, std::move(fields));
}

// Struct instantiation
// Point{ x: 10, y: 20 }
std::unique_ptr<StructInstanceExpr> Parser::structInstance()
{
	Token nameTok = consume(TokenType::Identifier, "Expected struct name");
	consume(TokenType::Symbol, "{", "Expected '{' in struct instance");

	std::vector<std::pair<std::string, std::unique_ptr<Expression>>> fields;
	while (!check(TokenType::Symbol) || peek().lexeme != "}")
	{
		if (isAtEnd())
			throw std::runtime_error("Unterminated struct instance.");

		Token fieldNameTok = consume(TokenType::Identifier, "Expected field name");
		consume(TokenType::Symbol, ":", "Expected ':' after field name");
		auto value = expression();
		fields.emplace_back(fieldNameTok.lexeme, std::move(value));

		if (!match(TokenType::Symbol, ","))
		{
			break;
		}
	}

	consume(TokenType::Symbol, "}", "Expected '}' after struct fields");
	return std::make_unique<StructInstanceExpr>(nameTok.lexeme, std::move(fields));
}

// Field access
// point.x
std::unique_ptr<Expression> Parser::fieldAccess(std::unique_ptr<Expression> object)
{
	Token nameTok = consume(TokenType::Identifier, "Expected field name after '.'");
	return std::make_unique<FieldAccessExpr>(std::move(object), nameTok.lexeme);
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

Token Parser::expect(TokenType type, const std::string& message) {
    if (check(type)) {
        return advance();
    }
    throw std::runtime_error(message);
}
