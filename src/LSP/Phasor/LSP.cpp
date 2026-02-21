#include "LSP.hpp"
#include <sstream>
#include <stdexcept>

namespace Phasor
{

static inline size_t toLexerLine(size_t lspLine)
{
	return lspLine + 1;
}
static inline size_t toLexerCol(size_t lspCol)
{
	return lspCol + 1;
}

static std::string buildSignature(AST::Node *node)
{
	if (auto *fn = dynamic_cast<AST::FunctionDecl *>(node))
	{
		std::ostringstream ss;
		ss << "fn " << fn->name << "(";
		for (size_t i = 0; i < fn->params.size(); ++i)
		{
			if (i)
				ss << ", ";
			ss << fn->params[i].name << ": ";
			if (fn->params[i].type)
			{
				ss << fn->params[i].type->name;
				if (fn->params[i].type->isPointer)
					ss << "*";
				for (int dim : fn->params[i].type->arrayDimensions)
					ss << "[" << dim << "]";
			}
		}
		ss << ")";
		if (fn->returnType)
		{
			ss << " -> " << fn->returnType->name;
			if (fn->returnType->isPointer)
				ss << "*";
		}
		return ss.str();
	}
	if (auto *st = dynamic_cast<AST::StructDecl *>(node))
	{
		std::ostringstream ss;
		ss << "struct " << st->name << " {";
		for (size_t i = 0; i < st->fields.size(); ++i)
		{
			if (i)
				ss << ", ";
			ss << " " << st->fields[i].name << ": ";
			if (st->fields[i].type)
			{
				ss << st->fields[i].type->name;
				if (st->fields[i].type->isPointer)
					ss << "*";
			}
		}
		ss << " }";
		return ss.str();
	}
	if (auto *vd = dynamic_cast<AST::VarDecl *>(node))
		return "var " + vd->name;

	return "";
}

void LSP::openDocument(const std::string &uri, const std::string &text)
{
	DocumentState doc;
	doc.uri = uri;
	doc.source = text;
	compile(doc);
	documents[uri] = std::move(doc);
}

void LSP::changeDocument(const std::string &uri, const std::string &newText)
{
	auto it = documents.find(uri);
	if (it == documents.end())
	{
		openDocument(uri, newText);
		return;
	}
	DocumentState &doc = it->second;
	doc.source = newText;
	doc.diagnostics.clear();
	doc.globalSymbols.clear();
	doc.program.reset();
	compile(doc);
}

void LSP::closeDocument(const std::string &uri)
{
	documents.erase(uri);
}

std::vector<LSP::Diagnostic> LSP::getDiagnostics(const std::string &uri) const
{
	auto it = documents.find(uri);
	if (it == documents.end())
		return {};
	return it->second.diagnostics;
}

AST::Node *LSP::findNodeAtPosition(const std::string &uri, size_t line, size_t column)
{
	auto it = documents.find(uri);
	if (it == documents.end() || !it->second.program)
		return nullptr;
	return walkForNode(it->second, toLexerLine(line), toLexerCol(column));
}

std::optional<std::string> LSP::getHover(const std::string &uri, size_t line, size_t column)
{
	auto it = documents.find(uri);
	if (it == documents.end() || !it->second.program)
		return std::nullopt;

	const DocumentState &doc = it->second;
	AST::Node           *node = walkForNode(doc, toLexerLine(line), toLexerCol(column));
	if (!node)
		return std::nullopt;

	std::string sig = buildSignature(node);
	if (!sig.empty())
		return sig;

	std::string name = symbolNameAt(node);
	if (name.empty())
		return std::nullopt;

	auto sit = doc.globalSymbols.find(name);
	if (sit == doc.globalSymbols.end())
		return std::nullopt;

	return sit->second.signature.empty() ? name : sit->second.signature;
}

std::optional<LSP::Location> LSP::getDefinition(const std::string &uri, size_t line, size_t column)
{
	auto it = documents.find(uri);
	if (it == documents.end() || !it->second.program)
		return std::nullopt;

	const DocumentState &doc = it->second;
	AST::Node           *node = walkForNode(doc, toLexerLine(line), toLexerCol(column));
	if (!node)
		return std::nullopt;

	std::string name = symbolNameAt(node);
	if (name.empty())
	{
		std::string sig = buildSignature(node);
		if (!sig.empty() && node->line > 0)
		{
			return Location{uri, node->line - 1, node->column > 0 ? node->column - 1 : 0};
		}
		return std::nullopt;
	}

	auto sit = doc.globalSymbols.find(name);
	if (sit == doc.globalSymbols.end())
		return std::nullopt;

	AST::Node *decl = sit->second.declaration;
	if (!decl || decl->line == 0)
		return std::nullopt;

	return Location{uri, decl->line - 1, decl->column > 0 ? decl->column - 1 : 0};
}

namespace
{
AST::Node *walkStmt(AST::Statement *stmt, size_t line, size_t col);
AST::Node *walkExpr(AST::Expression *expr, size_t line, size_t col);
AST::Node *candidate(AST::Node *node, size_t line, size_t col)
{
	if (!node || node->line == 0)
		return nullptr;
	if (node->line == line && node->column <= col)
		return node;
	return nullptr;
}

AST::Node *walkExpr(AST::Expression *expr, size_t line, size_t col)
{
	if (!expr)
		return nullptr;

	if (auto *e = dynamic_cast<AST::BinaryExpr *>(expr))
	{
		if (auto *n = walkExpr(e->left.get(), line, col))
			return n;
		if (auto *n = walkExpr(e->right.get(), line, col))
			return n;
	}
	else if (auto *e = dynamic_cast<AST::UnaryExpr *>(expr))
	{
		if (auto *n = walkExpr(e->operand.get(), line, col))
			return n;
	}
	else if (auto *e = dynamic_cast<AST::PostfixExpr *>(expr))
	{
		if (auto *n = walkExpr(e->operand.get(), line, col))
			return n;
	}
	else if (auto *e = dynamic_cast<AST::AssignmentExpr *>(expr))
	{
		if (auto *n = walkExpr(e->target.get(), line, col))
			return n;
		if (auto *n = walkExpr(e->value.get(), line, col))
			return n;
	}
	else if (auto *e = dynamic_cast<AST::CallExpr *>(expr))
	{
		for (const auto &arg : e->arguments)
			if (auto *n = walkExpr(arg.get(), line, col))
				return n;
	}
	else if (auto *e = dynamic_cast<AST::ArrayAccessExpr *>(expr))
	{
		if (auto *n = walkExpr(e->array.get(), line, col))
			return n;
		if (auto *n = walkExpr(e->index.get(), line, col))
			return n;
	}
	else if (auto *e = dynamic_cast<AST::ArrayLiteralExpr *>(expr))
	{
		for (const auto &elem : e->elements)
			if (auto *n = walkExpr(elem.get(), line, col))
				return n;
	}
	else if (auto *e = dynamic_cast<AST::MemberAccessExpr *>(expr))
	{
		if (auto *n = walkExpr(e->object.get(), line, col))
			return n;
	}
	else if (auto *e = dynamic_cast<AST::FieldAccessExpr *>(expr))
	{
		if (auto *n = walkExpr(e->object.get(), line, col))
			return n;
	}
	else if (auto *e = dynamic_cast<AST::StructInstanceExpr *>(expr))
	{
		for (const auto &fv : e->fieldValues)
			if (auto *n = walkExpr(fv.second.get(), line, col))
				return n;
	}

	return candidate(expr, line, col);
}

AST::Node *walkStmt(AST::Statement *stmt, size_t line, size_t col)
{
	if (!stmt)
		return nullptr;

	if (auto *s = dynamic_cast<AST::BlockStmt *>(stmt))
	{
		for (const auto &child : s->statements)
			if (auto *n = walkStmt(child.get(), line, col))
				return n;
	}
	else if (auto *s = dynamic_cast<AST::ExpressionStmt *>(stmt))
	{
		if (auto *n = walkExpr(s->expression.get(), line, col))
			return n;
	}
	else if (auto *s = dynamic_cast<AST::PrintStmt *>(stmt))
	{
		if (auto *n = walkExpr(s->expression.get(), line, col))
			return n;
	}
	else if (auto *s = dynamic_cast<AST::VarDecl *>(stmt))
	{
		if (auto *n = walkExpr(s->initializer.get(), line, col))
			return n;
		return candidate(s, line, col);
	}
	else if (auto *s = dynamic_cast<AST::ReturnStmt *>(stmt))
	{
		if (auto *n = walkExpr(s->value.get(), line, col))
			return n;
	}
	else if (auto *s = dynamic_cast<AST::IfStmt *>(stmt))
	{
		if (auto *n = walkExpr(s->condition.get(), line, col))
			return n;
		if (auto *n = walkStmt(s->thenBranch.get(), line, col))
			return n;
		if (auto *n = walkStmt(s->elseBranch.get(), line, col))
			return n;
	}
	else if (auto *s = dynamic_cast<AST::WhileStmt *>(stmt))
	{
		if (auto *n = walkExpr(s->condition.get(), line, col))
			return n;
		if (auto *n = walkStmt(s->body.get(), line, col))
			return n;
	}
	else if (auto *s = dynamic_cast<AST::ForStmt *>(stmt))
	{
		if (auto *n = walkStmt(s->initializer.get(), line, col))
			return n;
		if (auto *n = walkExpr(s->condition.get(), line, col))
			return n;
		if (auto *n = walkExpr(s->increment.get(), line, col))
			return n;
		if (auto *n = walkStmt(s->body.get(), line, col))
			return n;
	}
	else if (auto *s = dynamic_cast<AST::SwitchStmt *>(stmt))
	{
		if (auto *n = walkExpr(s->expr.get(), line, col))
			return n;
		for (const auto &c : s->cases)
		{
			if (auto *n = walkExpr(c.value.get(), line, col))
				return n;
			for (const auto &cs : c.statements)
				if (auto *n = walkStmt(cs.get(), line, col))
					return n;
		}
		for (const auto &ds : s->defaultStmts)
			if (auto *n = walkStmt(ds.get(), line, col))
				return n;
	}
	else if (auto *s = dynamic_cast<AST::FunctionDecl *>(stmt))
	{
		if (s->body)
			if (auto *n = walkStmt(s->body.get(), line, col))
				return n;
		return candidate(s, line, col);
	}
	else if (auto *s = dynamic_cast<AST::StructDecl *>(stmt))
	{
		return candidate(s, line, col);
	}
	else if (auto *s = dynamic_cast<AST::ExportStmt *>(stmt))
	{
		if (auto *n = walkStmt(s->declaration.get(), line, col))
			return n;
	}
	else if (auto *s = dynamic_cast<AST::UnsafeBlockStmt *>(stmt))
	{
		if (auto *n = walkStmt(s->block.get(), line, col))
			return n;
	}

	return candidate(stmt, line, col);
}
}

AST::Node *LSP::walkForNode(const DocumentState &doc, size_t line, size_t col)
{
	if (!doc.program)
		return nullptr;
	for (const auto &stmt : doc.program->statements)
		if (auto *n = walkStmt(stmt.get(), line, col))
			return n;
	return nullptr;
}

std::string LSP::symbolNameAt(AST::Node *node) const
{
	if (!node)
		return "";
	if (auto *e = dynamic_cast<AST::IdentifierExpr *>(node))
		return e->name;
	if (auto *e = dynamic_cast<AST::CallExpr *>(node))
		return e->callee;
	if (auto *e = dynamic_cast<AST::StructInstanceExpr *>(node))
		return e->structName;
	if (auto *e = dynamic_cast<AST::FieldAccessExpr *>(node))
		return e->fieldName;
	if (auto *e = dynamic_cast<AST::MemberAccessExpr *>(node))
		return e->member;
	return "";
}

void LSP::computeLineOffsets(DocumentState &doc)
{
	doc.lineStartOffsets.clear();
	doc.lineStartOffsets.push_back(0);
	for (size_t i = 0; i < doc.source.size(); ++i)
		if (doc.source[i] == '\n')
			doc.lineStartOffsets.push_back(i + 1);
}

void LSP::buildGlobalSymbols(DocumentState &doc)
{
	if (!doc.program)
		return;

	auto tryRegister = [&](const std::string &name, AST::Node *node) {
		SymbolInfo info;
		info.name = name;
		info.declaration = node;
		info.signature = buildSignature(node);
		doc.globalSymbols[name] = std::move(info);
	};

	for (const auto &stmt : doc.program->statements)
	{
		if (auto *fn = dynamic_cast<AST::FunctionDecl *>(stmt.get()))
			tryRegister(fn->name, fn);
		else if (auto *st = dynamic_cast<AST::StructDecl *>(stmt.get()))
			tryRegister(st->name, st);
		else if (auto *vd = dynamic_cast<AST::VarDecl *>(stmt.get()))
			tryRegister(vd->name, vd);
		else if (auto *ex = dynamic_cast<AST::ExportStmt *>(stmt.get()))
		{
			if (auto *fn = dynamic_cast<AST::FunctionDecl *>(ex->declaration.get()))
				tryRegister(fn->name, fn);
			else if (auto *st = dynamic_cast<AST::StructDecl *>(ex->declaration.get()))
				tryRegister(st->name, st);
			else if (auto *vd = dynamic_cast<AST::VarDecl *>(ex->declaration.get()))
				tryRegister(vd->name, vd);
		}
	}
}

void LSP::compile(DocumentState &doc)
{
	computeLineOffsets(doc);
	try
	{
		Lexer  lexer(doc.source);
		auto   tokens = lexer.tokenize();
		Parser parser(tokens);
		doc.program = parser.parse();

		if (auto err = lexer.getError())
		{
			doc.diagnostics.push_back({err->message, err->line, err->column, err->line, err->column + 1});
			return;
		}

		if (auto err = parser.getError())
		{
			doc.diagnostics.push_back({err->message, err->line, err->column, err->line, err->column + 1});
			doc.program.reset();
			return;
		}
	}
	catch (const std::runtime_error &e)
	{
		std::cerr << e.what();
	}
	catch (...)
	{
		std::cerr << "Caught unknown exception";
	}
	buildGlobalSymbols(doc);
}

} // namespace Phasor