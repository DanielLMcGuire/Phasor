#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace Phasor
{
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
	size_t      line;
	size_t      column;
};

/// @brief Abstract Syntax Tree (AST) namespace
namespace AST
{

// Forward declarations
struct Node;
struct Expression;
struct Statement;
struct Program;

/// @brief AST Node
struct Node
{
	virtual ~Node() = default;
	virtual void print(int indent = 0) const = 0;
};

/// @brief Expression Node
struct Expression : public Node
{
};

/// @brief Statement Node
struct Statement : public Node
{
};

/// @brief Program Node
struct Program : public Node
{
	std::vector<std::unique_ptr<Statement>> statements;
	void                                    print(int indent = 0) const override
	{
		for (const auto &stmt : statements)
		{
			stmt->print(indent);
		}
	}
};

/// @brief Type Node
struct TypeNode : public Node
{
	std::string name;
	bool        isPointer = false;
	std::vector<int> arrayDimensions;

	TypeNode(std::string n, bool ptr = false, std::vector<int> dims = {}) : name(n), isPointer(ptr), arrayDimensions(std::move(dims))
	{
	}
	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "Type: " << name << (isPointer ? "*" : "") << "\n";
		for (int dim : arrayDimensions) {
			std::cout << std::string(indent + 2, ' ') << "ArrayDim: " << dim << "\n";
		}
	}
};

/// @brief Numeral Expression Node
struct NumberExpr : public Expression
{
	std::string value;
	NumberExpr(std::string v) : value(v)
	{
	}
	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "Number: " << value << "\n";
	}
};

/// @brief String Expression Node
struct StringExpr : public Expression
{
	std::string value;
	StringExpr(std::string v) : value(v)
	{
	}
	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "String: " << value << "\n";
	}
};

/// @brief Identifier Expression Node
struct IdentifierExpr : public Expression
{
	std::string name;
	IdentifierExpr(std::string n) : name(n)
	{
	}
	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "Identifier: " << name << "\n";
	}
};

/// @brief Boolean Expression Node
struct BooleanExpr : public Expression
{
	bool value;
	BooleanExpr(bool v) : value(v)
	{
	}
	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "Boolean: " << (value ? "true" : "false") << "\n";
	}
};

/// @brief NULL Expression Node
struct NullExpr : public Expression
{
	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "Null\n";
	}
};

/// @brief Unary operator types
enum class UnaryOp
{
	Negate,     // -x
	Not,        // !x
	AddressOf,  // &x
	Dereference // *x
};

/// @brief Binary operator types
enum class BinaryOp
{
	Add,
	Subtract,
	Multiply,
	Divide,
	Modulo,
	And,
	Or,
	Equal,
	NotEqual,
	LessThan,
	GreaterThan,
	LessEqual,
	GreaterEqual
};

/// @brief Postfix operator types
enum class PostfixOp
{
	Increment, // x++
	Decrement  // x--
};

/// @brief Postfix Expression Node
struct PostfixExpr : public Expression
{
	PostfixOp                   op;
	std::unique_ptr<Expression> operand;

	PostfixExpr(PostfixOp o, std::unique_ptr<Expression> expr) : op(o), operand(std::move(expr))
	{
	}

	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "PostfixExpr: ";
		switch (op)
		{
		case PostfixOp::Increment:
			std::cout << "++\n";
			break;
		case PostfixOp::Decrement:
			std::cout << "--\n";
			break;
		}
		operand->print(indent + 2);
	}
};

/// @brief Unary Expression Node
struct UnaryExpr : public Expression
{
	UnaryOp                     op;
	std::unique_ptr<Expression> operand;

	UnaryExpr(UnaryOp o, std::unique_ptr<Expression> expr) : op(o), operand(std::move(expr))
	{
	}

	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "UnaryExpr: ";
		switch (op)
		{
		case UnaryOp::Negate:
			std::cout << "-\n";
			break;
		case UnaryOp::Not:
			std::cout << "!\n";
			break;
		case UnaryOp::AddressOf:
			std::cout << "&\n";
			break;
		case UnaryOp::Dereference:
			std::cout << "*\n";
			break;
		}
		operand->print(indent + 2);
	}
};

/// @brief Binary Expression Node
struct BinaryExpr : public Expression
{
	std::unique_ptr<Expression> left;
	BinaryOp                    op;
	std::unique_ptr<Expression> right;

	BinaryExpr(std::unique_ptr<Expression> l, BinaryOp o, std::unique_ptr<Expression> r)
	    : left(std::move(l)), op(o), right(std::move(r))
	{
	}

	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "BinaryExpr: ";
		switch (op)
		{
		case BinaryOp::Add:
			std::cout << "+\n";
			break;
		case BinaryOp::Subtract:
			std::cout << "-\n";
			break;
		case BinaryOp::Multiply:
			std::cout << "*\n";
			break;
		case BinaryOp::Divide:
			std::cout << "/\n";
			break;
		case BinaryOp::Modulo:
			std::cout << "%\n";
			break;
		case BinaryOp::And:
			std::cout << "&&\n";
			break;
		case BinaryOp::Or:
			std::cout << "||\n";
			break;
		case BinaryOp::Equal:
			std::cout << "==\n";
			break;
		case BinaryOp::NotEqual:
			std::cout << "!=\n";
			break;
		case BinaryOp::LessThan:
			std::cout << "<\n";
			break;
		case BinaryOp::GreaterThan:
			std::cout << ">\n";
			break;
		case BinaryOp::LessEqual:
			std::cout << "<=\n";
			break;
		case BinaryOp::GreaterEqual:
			std::cout << ">=\n";
			break;
		}
		left->print(indent + 2);
		right->print(indent + 2);
	}
};

/// @brief Array Access Expression Node
struct ArrayAccessExpr : public Expression
{
	std::unique_ptr<Expression> array;
	std::unique_ptr<Expression> index;

	ArrayAccessExpr(std::unique_ptr<Expression> arr, std::unique_ptr<Expression> idx)
	    : array(std::move(arr)), index(std::move(idx))
	{
	}

	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "ArrayAccess:\n";
		array->print(indent + 2);
		std::cout << std::string(indent + 2, ' ') << "Index:\n";
		index->print(indent + 4);
	}
};

/// @brief Array Literal Expression Node
struct ArrayLiteralExpr : public Expression
{
	std::vector<std::unique_ptr<Expression>> elements;
	ArrayLiteralExpr(std::vector<std::unique_ptr<Expression>> elems) : elements(std::move(elems)) {}
	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "ArrayLiteral:\n";
		for (const auto &elem : elements) elem->print(indent + 2);
	}
};

/// @brief Member Access Expression Node
struct MemberAccessExpr : public Expression
{
	std::unique_ptr<Expression> object;
	std::string                 member;

	MemberAccessExpr(std::unique_ptr<Expression> obj, std::string mem) : object(std::move(obj)), member(mem)
	{
	}

	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "MemberAccess: ." << member << "\n";
		object->print(indent + 2);
	}
};

/// @brief Call Expression Node
struct CallExpr : public Expression
{
	std::string                              callee;
	std::vector<std::unique_ptr<Expression>> arguments;

	CallExpr(std::string name, std::vector<std::unique_ptr<Expression>> args) : callee(name), arguments(std::move(args))
	{
	}

	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "CallExpr: " << callee << "\n";
		for (const auto &arg : arguments)
		{
			arg->print(indent + 2);
		}
	}
};

/// @brief Assignment Expression Node
struct AssignmentExpr : public Expression
{
	std::unique_ptr<Expression> target;
	std::unique_ptr<Expression> value;

	AssignmentExpr(std::unique_ptr<Expression> t, std::unique_ptr<Expression> v)
	    : target(std::move(t)), value(std::move(v))
	{
	}

	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "AssignmentExpr:\n";
		std::cout << std::string(indent + 2, ' ') << "Target:\n";
		target->print(indent + 4);
		std::cout << std::string(indent + 2, ' ') << "Value:\n";
		value->print(indent + 4);
	}
};

/// @brief Variable Declaration Node
struct VarDecl : public Statement
{
	std::string                 name;
	std::unique_ptr<Expression> initializer;
	VarDecl(std::string n, std::unique_ptr<Expression> init) : name(n), initializer(std::move(init))
	{
	}
	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "VarDecl: " << name << "\n";
		if (initializer)
			initializer->print(indent + 2);
	}
};

/// @brief Expression Statement Node
struct ExpressionStmt : public Statement
{
	std::unique_ptr<Expression> expression;
	ExpressionStmt(std::unique_ptr<Expression> expr) : expression(std::move(expr))
	{
	}
	void print(int indent = 0) const override
	{
		expression->print(indent);
	}
};

/// @brief Print Statement Node
struct PrintStmt : public Statement
{
	std::unique_ptr<Expression> expression;
	PrintStmt(std::unique_ptr<Expression> expr) : expression(std::move(expr))
	{
	}
	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "PrintStmt:\n";
		expression->print(indent + 2);
	}
};

/// @brief Import Statement Node
struct ImportStmt : public Statement
{
	std::string modulePath;
	ImportStmt(std::string path) : modulePath(path)
	{
	}
	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "ImportStmt: " << modulePath << "\n";
	}
};

/// @brief Export Statement Node
struct ExportStmt : public Statement
{
	std::unique_ptr<Statement> declaration;
	ExportStmt(std::unique_ptr<Statement> decl) : declaration(std::move(decl))
	{
	}
	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "ExportStmt:\n";
		declaration->print(indent + 2);
	}
};

/// @brief Block Statement Node
struct BlockStmt : public Statement
{
	std::vector<std::unique_ptr<Statement>> statements;
	BlockStmt(std::vector<std::unique_ptr<Statement>> stmts) : statements(std::move(stmts))
	{
	}
	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "BlockStmt:\n";
		for (const auto &stmt : statements)
		{
			stmt->print(indent + 2);
		}
	}
};

/// @brief Return Statement Node
struct ReturnStmt : public Statement
{
	std::unique_ptr<Expression> value;
	ReturnStmt(std::unique_ptr<Expression> val) : value(std::move(val))
	{
	}
	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "ReturnStmt:\n";
		if (value)
			value->print(indent + 2);
	}
};

/// @brief Break Statement Node
struct BreakStmt : public Statement
{
	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "BreakStmt\n";
	}
};

/// @brief Continue Statement Node
struct ContinueStmt : public Statement
{
	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "ContinueStmt\n";
	}
};

/// @brief If Statement Node
struct IfStmt : public Statement
{
	std::unique_ptr<Expression> condition;
	std::unique_ptr<Statement>  thenBranch;
	std::unique_ptr<Statement>  elseBranch;

	IfStmt(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> thenB, std::unique_ptr<Statement> elseB)
	    : condition(std::move(cond)), thenBranch(std::move(thenB)), elseBranch(std::move(elseB))
	{
	}

	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "IfStmt:\n";
		condition->print(indent + 2);
		thenBranch->print(indent + 2);
		if (elseBranch)
		{
			std::cout << std::string(indent, ' ') << "Else:\n";
			elseBranch->print(indent + 2);
		}
	}
};

/// @brief While Statement Node
struct WhileStmt : public Statement
{
	std::unique_ptr<Expression> condition;
	std::unique_ptr<Statement>  body;

	WhileStmt(std::unique_ptr<Expression> cond, std::unique_ptr<Statement> b)
	    : condition(std::move(cond)), body(std::move(b))
	{
	}

	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "WhileStmt:\n";
		condition->print(indent + 2);
		body->print(indent + 2);
	}
};

/// @brief For Statement Node
struct ForStmt : public Statement
{
	std::unique_ptr<Statement>  initializer;
	std::unique_ptr<Expression> condition;
	std::unique_ptr<Expression> increment;
	std::unique_ptr<Statement>  body;

	ForStmt(std::unique_ptr<Statement> init, std::unique_ptr<Expression> cond, std::unique_ptr<Expression> incr,
	        std::unique_ptr<Statement> b)
	    : initializer(std::move(init)), condition(std::move(cond)), increment(std::move(incr)), body(std::move(b))
	{
	}

	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "ForStmt:\n";
		if (initializer)
			initializer->print(indent + 2);
		if (condition)
			condition->print(indent + 2);
		if (increment)
			increment->print(indent + 2);
		body->print(indent + 2);
	}
};

/// @brief Unsafe Block Statement Node
struct UnsafeBlockStmt : public Statement
{
	std::unique_ptr<BlockStmt> block;
	UnsafeBlockStmt(std::unique_ptr<BlockStmt> b) : block(std::move(b))
	{
	}
	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "UnsafeBlockStmt:\n";
		block->print(indent + 2);
	}
};

/// @brief Function Declaration Node
struct FunctionDecl : public Statement
{
	std::string name;
	struct Param
	{
		std::string               name;
		std::unique_ptr<TypeNode> type;
	};
	std::vector<Param>         params;
	std::unique_ptr<TypeNode>  returnType;
	std::unique_ptr<BlockStmt> body;

	FunctionDecl(std::string n, std::vector<Param> p, std::unique_ptr<TypeNode> rt, std::unique_ptr<BlockStmt> b)
	    : name(n), params(std::move(p)), returnType(std::move(rt)), body(std::move(b))
	{
	}

	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "FunctionDecl: " << name << "\n";
		for (const auto &param : params)
		{
			std::cout << std::string(indent + 2, ' ') << "Param: " << param.name << " Type: ";
			param.type->print(0);
		}
		std::cout << std::string(indent + 2, ' ') << "Return Type: ";
		if (returnType)
			returnType->print(0);
		else
			std::cout << "void\n";
		if (body)
			body->print(indent + 2);
	}
};

/// @brief Struct Field Node
struct StructField
{
	std::string               name;
	std::unique_ptr<TypeNode> type;

	StructField(std::string n, std::unique_ptr<TypeNode> t) : name(std::move(n)), type(std::move(t))
	{
	}
};

/// @brief Struct Declaration Node
struct StructDecl : public Statement
{
	std::string              name;
	std::vector<StructField> fields;

	StructDecl(std::string n, std::vector<StructField> f) : name(std::move(n)), fields(std::move(f))
	{
	}

	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "StructDecl: " << name << "\n";
		for (const auto &field : fields)
		{
			std::cout << std::string(indent + 2, ' ') << field.name << ": ";
			field.type->print(0);
		}
	}
};

/// @brief Struct Instance Expression Node
struct StructInstanceExpr : public Expression
{
	std::string                                                      structName;
	std::vector<std::pair<std::string, std::unique_ptr<Expression>>> fieldValues;

	StructInstanceExpr(std::string name, std::vector<std::pair<std::string, std::unique_ptr<Expression>>> fields)
	    : structName(std::move(name)), fieldValues(std::move(fields))
	{
	}

	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "StructInstance: " << structName << "\n";
		for (const auto &field : fieldValues)
		{
			std::cout << std::string(indent + 2, ' ') << field.first << ":\n";
			field.second->print(indent + 4);
		}
	}
};

/// @brief Field Access Expression Node
struct FieldAccessExpr : public Expression
{
	std::unique_ptr<Expression> object;
	std::string                 fieldName;

	FieldAccessExpr(std::unique_ptr<Expression> obj, std::string field)
	    : object(std::move(obj)), fieldName(std::move(field))
	{
	}

	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "FieldAccess: ." << fieldName << "\n";
		object->print(indent + 2);
	}
};

/// @brief Case Clause Node
struct CaseClause
{
	std::unique_ptr<Expression>             value;
	std::vector<std::unique_ptr<Statement>> statements;

	CaseClause(std::unique_ptr<Expression> v, std::vector<std::unique_ptr<Statement>> stmts)
	    : value(std::move(v)), statements(std::move(stmts))
	{
	}
};

/// @brief Switch Statement Node
struct SwitchStmt : public Statement
{
	std::unique_ptr<Expression>             expr;
	std::vector<CaseClause>                 cases;
	std::vector<std::unique_ptr<Statement>> defaultStmts;

	SwitchStmt(std::unique_ptr<Expression> e, std::vector<CaseClause> c, std::vector<std::unique_ptr<Statement>> d)
	    : expr(std::move(e)), cases(std::move(c)), defaultStmts(std::move(d))
	{
	}

	void print(int indent = 0) const override
	{
		std::cout << std::string(indent, ' ') << "SwitchStmt:\n";
		expr->print(indent + 2);
		for (const auto &c : cases)
		{
			std::cout << std::string(indent + 2, ' ') << "case:\n";
			c.value->print(indent + 4);
			for (const auto &stmt : c.statements)
			{
				stmt->print(indent + 4);
			}
		}
		if (!defaultStmts.empty())
		{
			std::cout << std::string(indent + 2, ' ') << "default:\n";
			for (const auto &stmt : defaultStmts)
			{
				stmt->print(indent + 4);
			}
		}
	}
};
} // namespace AST
} // namespace Phasor