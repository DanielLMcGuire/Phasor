#include "Parser.hpp"
#include "../Lexer/Lexer.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <utility>

namespace Phasor
{

static std::set<std::filesystem::path> visitedFiles;
static int parseDepth = 0;

static std::vector<Token> tokenizeFile(const std::filesystem::path &path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file " + path.string());
	}
	std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();

	Lexer lexer(source);
	return lexer.tokenize();
}

static std::vector<std::unique_ptr<AST::Statement>> resolveIncludes(std::vector<std::unique_ptr<AST::Statement>> &stmts,
                                                                    const std::filesystem::path &baseDir,
                                                                    const std::vector<std::filesystem::path> &includePaths);

static std::vector<std::unique_ptr<AST::Statement>> resolveIncludesInternal(
    std::vector<std::unique_ptr<AST::Statement>> &stmts, const std::filesystem::path &baseDir,
    const std::vector<std::filesystem::path> &includePaths)
{
	std::vector<std::unique_ptr<AST::Statement>> result;

	for (auto &i : stmts)
	{
		if (auto *includeStmt = dynamic_cast<AST::IncludeStmt *>(i.get()))
		{
			auto includePath = (baseDir / includeStmt->modulePath).lexically_normal();
			if (!std::filesystem::exists(includePath))
			{
				bool found = false;
				for (const auto &incDir : includePaths)
				{
					auto tryPath = (incDir / includeStmt->modulePath).lexically_normal();
					if (std::filesystem::exists(tryPath))
					{
						includePath = tryPath;
						found = true;
						break;
					}
				}
				if (!found)
				{
					throw std::runtime_error("Could not find included file: " + includeStmt->modulePath.string());
				}
			}

			auto canonicalPath = std::filesystem::canonical(includePath);
			if (visitedFiles.count(canonicalPath) > 0)
			{
				continue;
			}
			visitedFiles.insert(canonicalPath);

			auto   tokens = tokenizeFile(canonicalPath);
			Parser parser(tokens, canonicalPath);
			parser.setIncludePaths(includePaths);
			auto   program = parser.parse();

			auto resolved = resolveIncludes(program->statements, canonicalPath.parent_path(), includePaths);
			for (auto &stmt : resolved)
			{
				result.push_back(std::move(stmt));
			}
		}
		else
		{
			result.push_back(std::move(i));
		}
	}

	return result;
}

static std::vector<std::unique_ptr<AST::Statement>> resolveIncludes(std::vector<std::unique_ptr<AST::Statement>> &stmts,
                                                                    const std::filesystem::path &baseDir,
                                                                    const std::vector<std::filesystem::path> &includePaths)
{
	return resolveIncludesInternal(stmts, baseDir, includePaths);
}

using namespace AST;

Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens)
{
}

Parser::Parser(const std::vector<Token> &tokens, std::filesystem::path sourcePath)
    : tokens(tokens), sourcePath(std::move(sourcePath))
{
}

std::unique_ptr<Program> Parser::parse()
{
	if (parseDepth == 0)
	{
		visitedFiles.clear();
		if (!sourcePath.empty())
		{
			visitedFiles.insert(std::filesystem::canonical(sourcePath));
		}
	}
	parseDepth++;

	struct DepthGuard
	{
		~DepthGuard()
		{
			parseDepth--;
		}
	} guard;

	auto program = std::make_unique<Program>();
	while (!isAtEnd())
	{
		program->statements.push_back(declaration());
	}

	program->statements = Phasor::resolveIncludes(program->statements, sourcePath.parent_path(), includePaths);

	return program;
}

std::unique_ptr<Statement> Parser::declaration()
{
	if (check(Phasor::TokenType::Keyword) && peek().lexeme == "fn")
	{
		Token start = peek();
		advance(); // consume 'fn'

		bool keepFunc = (check(Phasor::TokenType::Keyword)   && peek().lexeme == "keep") ||
		                (check(Phasor::TokenType::Identifier) && peek().lexeme == "keep");
		if (keepFunc) advance();

		Token nameTok = consume(Phasor::TokenType::Identifier, "Expect function name.");
		consume(Phasor::TokenType::Symbol, "(", "Expect '(' after function name.");
		
		std::vector<AST::Param> params;
		if (!check(Phasor::TokenType::Symbol) || peek().lexeme != ")")
		{
			do
			{
				Token paramName = consume(Phasor::TokenType::Identifier, "Expect parameter name.");
				consume(Phasor::TokenType::Symbol, ":", "Expect ':' after parameter name.");
				auto type = parseType();
				params.push_back({paramName.lexeme, std::move(type)});
			} while (match(Phasor::TokenType::Symbol, ","));
		}
		consume(Phasor::TokenType::Symbol, ")", "Expect ')' after parameters.");

		std::unique_ptr<TypeNode> returnType = nullptr;
		if (match(Phasor::TokenType::Symbol, "->"))
		{
			returnType = parseType();
		}

		if (match(Phasor::TokenType::Symbol, ";"))
		{
			auto node = std::make_unique<ForwardDecl>(nameTok.lexeme, std::move(params), std::move(returnType));
			node->line = start.line;
			node->column = start.column;
			return node;
		}
		else if (match(Phasor::TokenType::Symbol, "{"))
		{
			std::string previousFunction = currentFunction;
			currentFunction = nameTok.lexeme;
			auto body = block();
			currentFunction = previousFunction;

			auto node = std::make_unique<FunctionDecl>(nameTok.lexeme, std::move(params), std::move(returnType), std::move(body), keepFunc);
			node->line = start.line;
			node->column = start.column;
			return node;
		}
		else
		{
			lastError = {"Expect '{' or ';' after function signature.", peek().line, peek().column};
			throw std::runtime_error("Expect '{' or ';' after function signature.");
		}
	}

	if (check(Phasor::TokenType::Keyword) && peek().lexeme == "var")
	{
		Token start = peek();
		advance();
		auto node = varDeclaration();
		node->line = start.line;
		node->column = start.column;
		return node;
	}
	if (check(Phasor::TokenType::Keyword) && peek().lexeme == "import")
	{
		Token start = peek();
		advance();
		auto node = importStatement();
		node->line = start.line;
		node->column = start.column;
		return node;
	}
	if (check(Phasor::TokenType::Keyword) && peek().lexeme == "export")
	{
		Token start = peek();
		advance();
		auto node = exportStatement();
		node->line = start.line;
		node->column = start.column;
		return node;
	}
	if (check(Phasor::TokenType::Keyword) && peek().lexeme == "struct")
	{
		return structDecl();
	}
	return statement();
}

std::unique_ptr<TypeNode> Parser::parseType()
{
	Token start = peek();
	bool  isPointer = false;
	if (match(Phasor::TokenType::Symbol, "*"))
	{
		isPointer = true;
	}

	// Accept both identifiers and the 'any' keyword as type names
	Token typeName;
	if (check(Phasor::TokenType::Identifier))
	{
		typeName = consume(Phasor::TokenType::Identifier, "Expect type name.");
	}
	else if (check(Phasor::TokenType::Keyword) && peek().lexeme == "any")
	{
		typeName = advance();
	}
	else
	{
		typeName = consume(Phasor::TokenType::Identifier, "Expect type name.");
	}

	std::vector<int> dims;
	while (match(Phasor::TokenType::Symbol, "["))
	{
		if (check(Phasor::TokenType::Number))
		{
			Token size = consume(Phasor::TokenType::Number, "Expect array size in type declaration.");
			dims.push_back(std::stoi(size.lexeme));
		}
		else
		{
			dims.push_back(-1); 
		}
		consume(Phasor::TokenType::Symbol, "]", "Expect ']' after array size.");
	}
	auto node = std::make_unique<TypeNode>(typeName.lexeme, isPointer, dims);
	node->line = start.line;
	node->column = start.column;
	return node;
}

std::unique_ptr<Statement> Parser::varDeclaration()
{
	Token name = consume(Phasor::TokenType::Identifier, "Expect variable name.");

	if (!check(Phasor::TokenType::Symbol) || peek().lexeme != ":")
	{
		lastError = {"Variable '" + std::string(name.lexeme) + "' must have a type annotation (e.g. var " + std::string(name.lexeme) + ": int or var " + std::string(name.lexeme) + ": any)", name.line, name.column};
		throw std::runtime_error("Variable '" + std::string(name.lexeme) + "' must have a type annotation.");
	}
	consume(Phasor::TokenType::Symbol, ":", "Expect ':' after variable name.");
	auto type = parseType();

	std::unique_ptr<Expression> initializer = nullptr;
	if (match(Phasor::TokenType::Symbol, "="))
	{
		initializer = expression();
	}
	
	consume(Phasor::TokenType::Symbol, ";", "Expect ';' after variable declaration.");
	
	auto node = std::make_unique<VarDecl>(name.lexeme, std::move(type), std::move(initializer));
	node->line = name.line;
	node->column = name.column;
	return node;
}

std::unique_ptr<Statement> Parser::statement()
{
	if (check(Phasor::TokenType::Keyword) && peek().lexeme == "print")
	{
		Token start = peek();
		advance();
		auto node = printStatement();
		node->line = start.line;
		node->column = start.column;
		return node;
	}
	if (check(Phasor::TokenType::Keyword) && peek().lexeme == "if")
	{
		Token start = peek();
		advance();
		auto node = ifStatement();
		node->line = start.line;
		node->column = start.column;
		return node;
	}
	if (check(Phasor::TokenType::Keyword) && peek().lexeme == "while")
	{
		Token start = peek();
		advance();
		auto node = whileStatement();
		node->line = start.line;
		node->column = start.column;
		return node;
	}
	if (check(Phasor::TokenType::Keyword) && peek().lexeme == "for")
	{
		Token start = peek();
		advance();
		auto node = forStatement();
		node->line = start.line;
		node->column = start.column;
		return node;
	}
	if (check(Phasor::TokenType::Keyword) && peek().lexeme == "switch")
	{
		Token start = peek();
		advance();
		auto node = switchStatement();
		node->line = start.line;
		node->column = start.column;
		return node;
	}
	if (check(Phasor::TokenType::Keyword) && peek().lexeme == "return")
	{
		Token start = peek();
		advance();
		auto node = returnStatement();
		node->line = start.line;
		node->column = start.column;
		return node;
	}
	if (check(Phasor::TokenType::Keyword) && peek().lexeme == "break")
	{
		Token start = peek();
		advance();
		consume(Phasor::TokenType::Symbol, ";", "Expect ';' after 'break'.");
		auto node = std::make_unique<BreakStmt>();
		node->line = start.line;
		node->column = start.column;
		return node;
	}
	if (check(Phasor::TokenType::Keyword) && peek().lexeme == "continue")
	{
		Token start = peek();
		advance();
		consume(Phasor::TokenType::Symbol, ";", "Expect ';' after 'continue'.");
		auto node = std::make_unique<ContinueStmt>();
		node->line = start.line;
		node->column = start.column;
		return node;
	}
	if (check(Phasor::TokenType::Keyword) && peek().lexeme == "include")
	{
		Token start = peek();
		advance();

		Token pathToken = consume(Phasor::TokenType::String, "Expect file path after 'include'.");

		consume(Phasor::TokenType::Symbol, ";", "Expect ';' after include.");

		auto node = std::make_unique<IncludeStmt>(pathToken.lexeme);
		node->line = start.line;
		node->column = start.column;
		return node;
	}
	if (check(Phasor::TokenType::Keyword) && peek().lexeme == "unsafe")
	{
		Token start = peek();
		advance();
		auto node = unsafeStatement();
		node->line = start.line;
		node->column = start.column;
		return node;
	}
	if (check(Phasor::TokenType::Symbol) && peek().lexeme == "{")
	{
		Token start = peek();
		advance();
		auto blk = block();
		blk->line = start.line;
		blk->column = start.column;
		return blk;
	}
	return expressionStatement();
}

std::unique_ptr<Statement> Parser::ifStatement()
{
	consume(Phasor::TokenType::Symbol, "(", "Expect '(' after 'if'.");
	auto condition = expression();
	consume(Phasor::TokenType::Symbol, ")", "Expect ')' after if condition.");
	auto                       thenBranch = statement();
	std::unique_ptr<Statement> elseBranch = nullptr;
	if (match(Phasor::TokenType::Keyword, "else"))
	{
		elseBranch = statement();
	}
	return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Statement> Parser::whileStatement()
{
	consume(Phasor::TokenType::Symbol, "(", "Expect '(' after 'while'.");
	auto condition = expression();
	consume(Phasor::TokenType::Symbol, ")", "Expect ')' after while condition.");
	auto body = statement();
	return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<Statement> Parser::forStatement()
{
	consume(Phasor::TokenType::Symbol, "(", "Expect '(' after 'for'.");

	std::unique_ptr<Statement> initializer = nullptr;
	if (!check(Phasor::TokenType::Symbol) || peek().lexeme != ";")
	{
		if (check(Phasor::TokenType::Keyword) && peek().lexeme == "var")
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
		consume(Phasor::TokenType::Symbol, ";", "Expect ';'.");
	}

	std::unique_ptr<Expression> condition = nullptr;
	if (!check(Phasor::TokenType::Symbol) || peek().lexeme != ";")
	{
		condition = expression();
	}
	consume(Phasor::TokenType::Symbol, ";", "Expect ';' after loop condition.");

	std::unique_ptr<Expression> increment = nullptr;
	if (!check(Phasor::TokenType::Symbol) || peek().lexeme != ")")
	{
		increment = expression();
	}
	consume(Phasor::TokenType::Symbol, ")", "Expect ')' after for clauses.");

	auto body = statement();
	return std::make_unique<ForStmt>(std::move(initializer), std::move(condition), std::move(increment),
	                                 std::move(body));
}

std::unique_ptr<Statement> Parser::switchStatement()
{
	consume(Phasor::TokenType::Symbol, "(", "Expect '(' after 'switch'.");
	auto expr = expression();
	consume(Phasor::TokenType::Symbol, ")", "Expect ')' after switch expression.");
	consume(Phasor::TokenType::Symbol, "{", "Expect '{' after switch.");

	std::vector<CaseClause>                 cases;
	std::vector<std::unique_ptr<Statement>> defaultStmts;

	while (!check(Phasor::TokenType::Symbol) || peek().lexeme != "}")
	{
		if (isAtEnd())
		{
			lastError = {"Unterminated switch statement.", peek().line, peek().column};
			throw std::runtime_error("Unterminated switch statement.");
		}

		if (check(Phasor::TokenType::Keyword) && peek().lexeme == "case")
		{
			advance();
			auto caseValue = expression();
			consume(Phasor::TokenType::Symbol, ":", "Expect ':' after case value.");

			std::vector<std::unique_ptr<Statement>> stmts;
			while ((!check(Phasor::TokenType::Keyword) || (peek().lexeme != "case" && peek().lexeme != "default")) &&
			       (!check(Phasor::TokenType::Symbol) || peek().lexeme != "}"))
			{
				if (isAtEnd())
				{
					lastError = {"Unterminated case clause.", peek().line, peek().column};
					throw std::runtime_error("Unterminated case clause.");
				}
				stmts.push_back(declaration());
			}
			cases.emplace_back(std::move(caseValue), std::move(stmts));
		}
		else if (check(Phasor::TokenType::Keyword) && peek().lexeme == "default")
		{
			advance();
			consume(Phasor::TokenType::Symbol, ":", "Expect ':' after default.");

			while ((!check(Phasor::TokenType::Keyword) || (peek().lexeme != "case" && peek().lexeme != "default")) &&
			       (!check(Phasor::TokenType::Symbol) || peek().lexeme != "}"))
			{
				if (isAtEnd())
				{
					lastError = {"Unterminated default clause.", peek().line, peek().column};
					throw std::runtime_error("Unterminated default clause.");
				}
				defaultStmts.push_back(declaration());
			}
		}
		else
		{
			lastError = {"Expected 'case' or 'default' in switch statement.", peek().line, peek().column};
			throw std::runtime_error("Expected 'case' or 'default' in switch statement.");
		}
	}

	consume(Phasor::TokenType::Symbol, "}", "Expect '}' after switch body.");
	return std::make_unique<SwitchStmt>(std::move(expr), std::move(cases), std::move(defaultStmts));
}

std::unique_ptr<Statement> Parser::returnStatement()
{
	std::unique_ptr<Expression> value = nullptr;
	if (!check(Phasor::TokenType::Symbol) || peek().lexeme != ";")
	{
		value = expression();
	}
	consume(Phasor::TokenType::Symbol, ";", "Expect ';' after return value.");
	return std::make_unique<ReturnStmt>(std::move(value));
}

std::unique_ptr<Statement> Parser::unsafeStatement()
{
	consume(Phasor::TokenType::Symbol, "{", "Expect '{' after 'unsafe'.");
	return std::make_unique<UnsafeBlockStmt>(block());
}

std::unique_ptr<BlockStmt> Parser::block()
{
	std::vector<std::unique_ptr<Statement>> statements;
	while (!check(Phasor::TokenType::Symbol) || peek().lexeme != "}")
	{
		if (isAtEnd())
		{
			lastError = {"Unterminated block.", peek().line, peek().column};
			throw std::runtime_error("Unterminated block.");
		}
		statements.push_back(declaration());
	}
	consume(Phasor::TokenType::Symbol, "}", "Expect '}' after block.");
	return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Statement> Parser::printStatement()
{
	auto expr = expression();
	consume(Phasor::TokenType::Symbol, ";", "Expect ';' after print statement.");
	return std::make_unique<PrintStmt>(std::move(expr));
}

std::unique_ptr<Statement> Parser::importStatement()
{
	Token path = consume(Phasor::TokenType::String, "Expect string after 'import'.");
	consume(Phasor::TokenType::Symbol, ";", "Expect ';' after import statement.");
	auto node = std::make_unique<ImportStmt>(path.lexeme);
	node->line = path.line;
	node->column = path.column;
	return node;
}

std::unique_ptr<Statement> Parser::exportStatement()
{
	auto node = std::make_unique<ExportStmt>(declaration());
	return node;
}

std::unique_ptr<Statement> Parser::expressionStatement()
{
	Token start = peek();
	auto  expr = expression();
	consume(Phasor::TokenType::Symbol, ";", "Expect ';' after expression.");
	auto node = std::make_unique<ExpressionStmt>(std::move(expr));
	node->line = start.line;
	node->column = start.column;
	return node;
}

std::unique_ptr<Expression> Parser::expression()
{
	return assignment();
}

std::unique_ptr<Expression> Parser::assignment()
{
	auto expr = logicalOr();

	if (match(Phasor::TokenType::Symbol, "="))
	{
		Token op = previous();
		auto  value = assignment();
		auto  node = std::make_unique<AssignmentExpr>(std::move(expr), std::move(value));
		node->line = op.line;
		node->column = op.column;
		return node;
	}

	return expr;
}

std::unique_ptr<Expression> Parser::logicalOr()
{
	auto expr = logicalAnd();

	while (check(Phasor::TokenType::Symbol) && peek().lexeme == "||")
	{
		Token op = advance();
		auto  right = logicalAnd();
		auto  node = std::make_unique<BinaryExpr>(std::move(expr), BinaryOp::Or, std::move(right));
		node->line = op.line;
		node->column = op.column;
		expr = std::move(node);
	}

	return expr;
}

std::unique_ptr<Expression> Parser::logicalAnd()
{
	auto expr = equality();

	while (check(Phasor::TokenType::Symbol) && peek().lexeme == "&&")
	{
		Token op = advance();
		auto  right = equality();
		auto  node = std::make_unique<BinaryExpr>(std::move(expr), BinaryOp::And, std::move(right));
		node->line = op.line;
		node->column = op.column;
		expr = std::move(node);
	}

	return expr;
}

std::unique_ptr<Expression> Parser::equality()
{
	auto expr = comparison();

	while (check(Phasor::TokenType::Symbol) && (peek().lexeme == "==" || peek().lexeme == "!="))
	{
		Token    op = advance();
		auto     right = comparison();
		BinaryOp binOp = (op.lexeme == "==") ? BinaryOp::Equal : BinaryOp::NotEqual;
		auto     node = std::make_unique<BinaryExpr>(std::move(expr), binOp, std::move(right));
		node->line = op.line;
		node->column = op.column;
		expr = std::move(node);
	}

	return expr;
}

std::unique_ptr<Expression> Parser::comparison()
{
	auto expr = term();

	while (check(Phasor::TokenType::Symbol) &&
	       (peek().lexeme == "<" || peek().lexeme == ">" || peek().lexeme == "<=" || peek().lexeme == ">="))
	{
		Token    op = advance();
		auto     right = term();
		BinaryOp binOp;
		if (op.lexeme == "<")
		{
			binOp = BinaryOp::LessThan;
		}
		else if (op.lexeme == ">")
		{
			binOp = BinaryOp::GreaterThan;
		}
		else if (op.lexeme == "<=")
		{
			binOp = BinaryOp::LessEqual;
		}
		else
		{
			binOp = BinaryOp::GreaterEqual;
		}

		auto node = std::make_unique<BinaryExpr>(std::move(expr), binOp, std::move(right));
		node->line = op.line;
		node->column = op.column;
		expr = std::move(node);
	}

	return expr;
}

std::unique_ptr<Expression> Parser::term()
{
	auto expr = factor();

	while (check(Phasor::TokenType::Symbol) && (peek().lexeme == "+" || peek().lexeme == "-"))
	{
		Token    op = advance();
		auto     right = factor();
		BinaryOp binOp = (op.lexeme == "+") ? BinaryOp::Add : BinaryOp::Subtract;
		auto     node = std::make_unique<BinaryExpr>(std::move(expr), binOp, std::move(right));
		node->line = op.line;
		node->column = op.column;
		expr = std::move(node);
	}

	return expr;
}

std::unique_ptr<Expression> Parser::factor()
{
	auto expr = unary();

	while (check(Phasor::TokenType::Symbol) && (peek().lexeme == "*" || peek().lexeme == "/" || peek().lexeme == "%"))
	{
		Token    op = advance();
		auto     right = unary();
		BinaryOp binOp;
		if (op.lexeme == "*")
		{
			binOp = BinaryOp::Multiply;
		}
		else if (op.lexeme == "/")
		{
			binOp = BinaryOp::Divide;
		}
		else
		{
			binOp = BinaryOp::Modulo;
		}

		auto node = std::make_unique<BinaryExpr>(std::move(expr), binOp, std::move(right));
		node->line = op.line;
		node->column = op.column;
		expr = std::move(node);
	}

	return expr;
}

std::unique_ptr<Expression> Parser::unary()
{
	if (check(Phasor::TokenType::Symbol) && (peek().lexeme == "!" || peek().lexeme == "-"))
	{
		Token   op = advance();
		auto    right = unary();
		UnaryOp uOp = (op.lexeme == "!") ? UnaryOp::Not : UnaryOp::Negate;
		auto    node = std::make_unique<UnaryExpr>(uOp, std::move(right));
		node->line = op.line;
		node->column = op.column;
		return node;
	}
	if (check(Phasor::TokenType::Symbol) && peek().lexeme == "&")
	{
		Token op = advance();
		auto  right = unary();
		auto  node = std::make_unique<UnaryExpr>(UnaryOp::AddressOf, std::move(right));
		node->line = op.line;
		node->column = op.column;
		return node;
	}
	if (check(Phasor::TokenType::Symbol) && peek().lexeme == "*")
	{
		Token op = advance();
		auto  right = unary();
		auto  node = std::make_unique<UnaryExpr>(UnaryOp::Dereference, std::move(right));
		node->line = op.line;
		node->column = op.column;
		return node;
	}
	return call();
}

std::unique_ptr<Expression> Parser::call()
{
	auto expr = primary();

	while (true)
	{
		if (match(Phasor::TokenType::Symbol, "("))
		{
			Token op = previous();
			expr = finishCall(std::move(expr));
			expr->line = op.line;
			expr->column = op.column;
		}
		else if (match(Phasor::TokenType::Symbol, "."))
		{
			Token op = previous();
			expr = fieldAccess(std::move(expr));
			expr->line = op.line;
			expr->column = op.column;
		}
		else if (match(Phasor::TokenType::Symbol, "++"))
		{
			Token op = previous();
			auto  node = std::make_unique<PostfixExpr>(PostfixOp::Increment, std::move(expr));
			node->line = op.line;
			node->column = op.column;
			expr = std::move(node);
		}
		else if (match(Phasor::TokenType::Symbol, "--"))
		{
			Token op = previous();
			auto  node = std::make_unique<PostfixExpr>(PostfixOp::Decrement, std::move(expr));
			node->line = op.line;
			node->column = op.column;
			expr = std::move(node);
		}
		else if (match(Phasor::TokenType::Symbol, "["))
		{
			Token op = previous();
			auto  index = expression();
			consume(Phasor::TokenType::Symbol, "]", "Expect ']' after index.");

			if (auto* strLit = dynamic_cast<StringExpr*>(index.get()))
			{
				auto node = std::make_unique<FieldAccessExpr>(std::move(expr), strLit->value);
				node->line = op.line;
				node->column = op.column;
				expr = std::move(node);
			}
			else
			{
				auto node = std::make_unique<ArrayAccessExpr>(std::move(expr), std::move(index));
				node->line = op.line;
				node->column = op.column;
				expr = std::move(node);
			}
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
	if (!check(Phasor::TokenType::Symbol) || peek().lexeme != ")")
	{
		do
		{
			arguments.push_back(expression());
		} while (match(Phasor::TokenType::Symbol, ","));
	}
	consume(Phasor::TokenType::Symbol, ")", "Expect ')' after arguments.");

	if (auto *ident = dynamic_cast<IdentifierExpr *>(callee.get()))
	{
		auto node = std::make_unique<CallExpr>(ident->name, std::move(arguments));
		node->line = ident->line;
		node->column = ident->column;
		return node;
	}
	if (auto field = dynamic_cast<FieldAccessExpr *>(callee.get()))
	{
		std::string methodName = field->fieldName;
		size_t      fline = field->line, fcol = field->column;
		arguments.insert(arguments.begin(), std::move(field->object));
		auto node = std::make_unique<CallExpr>(methodName, std::move(arguments));
		node->line = fline;
		node->column = fcol;
		return node;
	}
	lastError = {"Can only call named functions.", peek().line, peek().column};
	throw std::runtime_error("Can only call named functions.");
}

std::unique_ptr<Expression> Parser::primary()
{
	if (match(Phasor::TokenType::Number))
	{
		Token t = previous();
		auto  node = std::make_unique<NumberExpr>(t.lexeme);
		node->line = t.line;
		node->column = t.column;
		return node;
	}
	if (match(Phasor::TokenType::String))
	{
		Token t = previous();
		auto  node = std::make_unique<StringExpr>(t.lexeme);
		node->line = t.line;
		node->column = t.column;
		return node;
	}
	if (check(Phasor::TokenType::Identifier))
	{
		Token identTok = peek();
		if (!isAtEnd() && peekNext().type == Phasor::TokenType::Symbol && peekNext().lexeme == "{")
		{
			return structInstance();
		}
		advance();
		auto node = std::make_unique<IdentifierExpr>(identTok.lexeme);
		node->line = identTok.line;
		node->column = identTok.column;
		return node;
	}
	if (match(Phasor::TokenType::Symbol, "["))
	{
		Token                                    start = previous();
		std::vector<std::unique_ptr<Expression>> elements;
		if (!check(Phasor::TokenType::Symbol) || peek().lexeme != "]")
		{
			do
			{
				elements.push_back(expression());
			} while (match(Phasor::TokenType::Symbol, ","));
		}
		consume(Phasor::TokenType::Symbol, "]", "Expect ']' after array elements.");
		auto node = std::make_unique<ArrayLiteralExpr>(std::move(elements));
		node->line = start.line;
		node->column = start.column;
		return node;
	}
	if (match(Phasor::TokenType::Keyword, "true"))
	{
		Token t = previous();
		auto  node = std::make_unique<BooleanExpr>(true);
		node->line = t.line;
		node->column = t.column;
		return node;
	}
	if (match(Phasor::TokenType::Keyword, "false"))
	{
		Token t = previous();
		auto  node = std::make_unique<BooleanExpr>(false);
		node->line = t.line;
		node->column = t.column;
		return node;
	}
	if (match(Phasor::TokenType::Keyword, "null"))
	{
		Token t = previous();
		auto  node = std::make_unique<NullExpr>();
		node->line = t.line;
		node->column = t.column;
		return node;
	}
	if (match(Phasor::TokenType::Symbol, "("))
	{
		auto expr = expression();
		consume(Phasor::TokenType::Symbol, ")", "Expect ')' after expression.");
		return expr;
	}
	if (check(Phasor::TokenType::Symbol) && peek().lexeme == "{")
	{
		if (current + 1 < static_cast<int>(tokens.size()) &&
			tokens[current + 1].type == Phasor::TokenType::Symbol &&
			tokens[current + 1].lexeme == "}")
		{
			return anonymousStructInstance();
		}
		if (current + 2 < static_cast<int>(tokens.size()) &&
			tokens[current + 1].type == Phasor::TokenType::Identifier &&
			tokens[current + 2].type == Phasor::TokenType::Symbol &&
			tokens[current + 2].lexeme == ":")
		{
			return anonymousStructInstance();
		}
	}
	std::cerr << "Error: Expect expression at '" << peek().lexeme << "'";
	std::cerr << " (line " << peek().line << ", column " << peek().column << ")\n";
	lastError = {"Expect expression", peek().line, peek().column};
	throw std::runtime_error("Expect expression.");
}

std::unique_ptr<StructDecl> Parser::structDecl()
{
	Token start = peek();
	consume(Phasor::TokenType::Keyword, "struct", "Expected 'struct'");
	Token nameTok = consume(Phasor::TokenType::Identifier, "Expected struct name");
	consume(Phasor::TokenType::Symbol, "{", "Expected '{' in struct declaration");

	std::vector<StructField> fields;
	while (!check(Phasor::TokenType::Symbol) || peek().lexeme != "}")
	{
		if (isAtEnd())
		{
			lastError = {"Unterminated struct declaration", peek().line, peek().column};
			throw std::runtime_error("Unterminated struct declaration.");
		}

		Token fieldNameTok = consume(Phasor::TokenType::Identifier, "Expected field name");
		if (!check(Phasor::TokenType::Symbol) || peek().lexeme != ":")
		{
			lastError = {"Struct field '" + std::string(fieldNameTok.lexeme) + "' must have a type annotation (e.g. " + std::string(fieldNameTok.lexeme) + ": int or " + std::string(fieldNameTok.lexeme) + ": any[])", fieldNameTok.line, fieldNameTok.column};
			throw std::runtime_error("Struct field '" + std::string(fieldNameTok.lexeme) + "' must have a type annotation.");
		}
		consume(Phasor::TokenType::Symbol, ":", "Expected ':' after field name");
		auto type = parseType();
		fields.emplace_back(fieldNameTok.lexeme, std::move(type));

		if (!match(Phasor::TokenType::Symbol, ","))
		{
			break;
		}
	}

	consume(Phasor::TokenType::Symbol, "}", "Expected '}' after struct fields");
	auto node = std::make_unique<StructDecl>(nameTok.lexeme, std::move(fields));
	node->line = start.line;
	node->column = start.column;
	return node;
}

std::unique_ptr<AST::StructInstanceExpr> Parser::structInstance()
{
	Token nameTok = consume(Phasor::TokenType::Identifier, "Expected struct name");
	consume(Phasor::TokenType::Symbol, "{", "Expected '{' in struct instance");

	std::vector<std::pair<std::string, std::unique_ptr<Expression>>> fields;
	while (!check(Phasor::TokenType::Symbol) || peek().lexeme != "}")
	{
		if (isAtEnd())
		{
			lastError = {"Unterminated struct instance.", peek().line, peek().column};
			throw std::runtime_error("Unterminated struct instance.");
		}
		Token fieldNameTok = consume(Phasor::TokenType::Identifier, "Expected field name");
		consume(Phasor::TokenType::Symbol, ":", "Expected ':' after field name");
		auto value = expression();
		fields.emplace_back(fieldNameTok.lexeme, std::move(value));

		if (!match(Phasor::TokenType::Symbol, ","))
		{
			break;
		}
	}

	consume(Phasor::TokenType::Symbol, "}", "Expected '}' after struct fields");
	auto node = std::make_unique<AST::StructInstanceExpr>(nameTok.lexeme, std::move(fields));
	node->line = nameTok.line;
	node->column = nameTok.column;
	return node;
}

std::unique_ptr<AST::StructInstanceExpr> Parser::anonymousStructInstance()
{
	Token start = peek();
	consume(Phasor::TokenType::Symbol, "{", "Expected '{' in anonymous struct literal.");

	std::vector<std::pair<std::string, std::unique_ptr<Expression>>> fields;
	while (!check(Phasor::TokenType::Symbol) || peek().lexeme != "}")
	{
		if (isAtEnd())
		{
			lastError = {"Unterminated anonymous struct literal.", peek().line, peek().column};
			throw std::runtime_error("Unterminated anonymous struct literal.");
		}
		Token fieldNameTok = consume(Phasor::TokenType::Identifier, "Expected field name in anonymous struct.");
		consume(Phasor::TokenType::Symbol, ":", "Expected ':' after field name.");
		auto value = expression();
		fields.emplace_back(fieldNameTok.lexeme, std::move(value));

		if (!match(Phasor::TokenType::Symbol, ","))
			break;
	}
	consume(Phasor::TokenType::Symbol, "}", "Expected '}' after anonymous struct fields.");

	auto node = std::make_unique<AST::StructInstanceExpr>("__anon", std::move(fields));
	node->line = start.line;
	node->column = start.column;
	return node;
}

std::unique_ptr<Expression> Parser::fieldAccess(std::unique_ptr<Expression> object)
{
	Token nameTok = consume(Phasor::TokenType::Identifier, "Expected field name after '.'");
	auto  node = std::make_unique<FieldAccessExpr>(std::move(object), nameTok.lexeme);
	node->line = nameTok.line;
	node->column = nameTok.column;
	return node;
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
	{
		current++;
	}
	return previous();
}

bool Parser::isAtEnd()
{
	return peek().type == Phasor::TokenType::EndOfFile;
}

bool Parser::check(Phasor::TokenType type)
{
	if (isAtEnd())
	{
		return false;
	}
	return peek().type == type;
}

bool Parser::match(Phasor::TokenType type)
{
	if (check(type))
	{
		advance();
		return true;
	}
	return false;
}

Token Parser::consume(Phasor::TokenType type, const std::string &message)
{
	if (check(type))
	{
		return advance();
	}

	std::cerr << "Error: " << message << " at '" << peek().lexeme << "'";
	std::cerr << " (line " << peek().line << ", column " << peek().column << ")";
	if (!currentFunction.empty())
	{
		std::cerr << " [in function '" << currentFunction << "']";
	}
	std::cerr << "\n";
	lastError = {message, peek().line, peek().column};
	throw std::runtime_error(message);
}

bool Parser::match(Phasor::TokenType type, const std::string &lexeme)
{
	if (check(type) && peek().lexeme == lexeme)
	{
		advance();
		return true;
	}
	return false;
}

Token Parser::consume(Phasor::TokenType type, const std::string &lexeme, const std::string &message)
{
	if (check(type) && peek().lexeme == lexeme)
	{
		return advance();
	}

	std::cerr << "Error: " << message << " at '" << peek().lexeme << "'";
	std::cerr << " (line " << peek().line << ", column " << peek().column << ")";
	if (!currentFunction.empty())
	{
		std::cerr << " [in function '" << currentFunction << "']";
	}
	std::cerr << "\n";

	lastError = {message, peek().line, peek().column};
	throw std::runtime_error(message);
}

Token Parser::expect(Phasor::TokenType type, const std::string &message)
{
	if (check(type))
	{
		return advance();
	}
	lastError = {message, peek().line, peek().column};
	throw std::runtime_error(message);
}
} // namespace Phasor