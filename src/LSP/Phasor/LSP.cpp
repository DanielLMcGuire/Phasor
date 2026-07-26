#include "LSP.hpp"
#include <algorithm>
#include <cctype>
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
static inline size_t toLspLine(size_t lexerLine)
{
	return lexerLine > 0 ? lexerLine - 1 : 0;
}
static inline size_t toLspColumn(size_t lexerColumn)
{
	return lexerColumn > 0 ? lexerColumn - 1 : 0;
}

std::filesystem::path LSP::uriToPath(const std::string &uri)
{
	static const std::string filePrefix = "file://";
	if (uri.compare(0, filePrefix.size(), filePrefix) != 0)
		return {};

	std::string rest = uri.substr(filePrefix.size());
	std::string decoded;
	decoded.reserve(rest.size());
	auto hexVal = [](char c) -> int {
		if (c >= '0' && c <= '9') return c - '0';
		if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
		if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
		return -1;
	};
	for (size_t i = 0; i < rest.size(); ++i)
	{
		if (rest[i] == '%' && i + 2 < rest.size())
		{
			int hi = hexVal(rest[i + 1]);
			int lo = hexVal(rest[i + 2]);
			if (hi >= 0 && lo >= 0)
			{
				decoded += static_cast<char>((hi << 4) | lo);
				i += 2;
				continue;
			}
		}
		decoded += rest[i];
	}

#if defined(_WIN32)
	if (decoded.size() > 2 && decoded[0] == '/' && decoded[2] == ':')
	{
		decoded.erase(0, 1);
	}
#endif

	return std::filesystem::path(decoded);
}

bool LSP::Scope::contains(size_t line, size_t column) const
{
	auto lessEq = [](size_t l1, size_t c1, size_t l2, size_t c2) { return l1 != l2 ? l1 < l2 : c1 <= c2; };
	return lessEq(startLine, startColumn, line, column) && lessEq(line, column, endLine, endColumn);
}

namespace
{

bool lessOrEqualPos(size_t l1, size_t c1, size_t l2, size_t c2)
{
	return l1 != l2 ? l1 < l2 : c1 <= c2;
}

LSP::Scope *innermostScopeAt(LSP::Scope *scope, size_t line, size_t col)
{
	if (!scope)
	{
		return nullptr;
	}
	for (auto &child : scope->children)
	{
		if (child->contains(line, col))
		{
			return innermostScopeAt(child.get(), line, col);
		}
	}
	return scope;
}

LSP::SymbolInfo *resolveInScope(LSP::Scope *from, const std::string &name, size_t line, size_t col)
{
	for (LSP::Scope *s = from; s != nullptr; s = s->parent)
	{
		auto it = s->symbols.find(name);
		if (it != s->symbols.end())
		{
			bool isRoot = (s->parent == nullptr);
			if (isRoot || lessOrEqualPos(it->second.declLine, it->second.declColumn, line, col))
			{
				return &it->second;
			}
		}
	}
	return nullptr;
}

std::string formatTypeNode(const AST::TypeNode *t)
{
	if (!t)
	{
		return "void";
	}
	std::string s = t->name;
	if (t->isPointer)
	{
		s += "*";
	}
	for (int dim : t->arrayDimensions)
	{
		s += dim > 0 ? ("[" + std::to_string(dim) + "]") : "[]";
	}
	return s;
}

LSP::SymbolInfo *declareSymbol(LSP::Scope *scope, LSP::SymbolInfo info)
{
	std::string name = info.name;
	scope->symbols[name] = std::move(info);
	return &scope->symbols[name];
}

void addOccurrence(LSP::DocumentState &doc, size_t line, size_t col, const std::string &name, AST::Node *node,
                    bool isDecl, LSP::SymbolInfo *sym)
{
	if (line == 0 || name.empty())
	{
		return;
	}
	LSP::Occurrence occ;
	occ.line = line;
	occ.startColumn = col;
	occ.endColumn = col + name.size();
	occ.name = name;
	occ.node = node;
	occ.isDeclaration = isDecl;
	occ.symbol = sym;
	doc.occurrences.push_back(std::move(occ));
}

std::string inferTypeName(LSP::DocumentState &doc, AST::Expression *expr, LSP::Scope *scope);
void        bindType(LSP::DocumentState &doc, AST::TypeNode *t, LSP::Scope *scope);
void        bindExpr(LSP::DocumentState &doc, AST::Expression *expr, LSP::Scope *scope);
void        bindStmt(LSP::DocumentState &doc, AST::Statement *stmt, LSP::Scope *scope);
void        bindStmts(LSP::DocumentState &doc, std::vector<std::unique_ptr<AST::Statement>> &stmts,
                       LSP::Scope *scope);

std::string inferTypeName(LSP::DocumentState &doc, AST::Expression *expr, LSP::Scope *scope)
{
	if (!expr)
	{
		return "";
	}
	if (auto *id = dynamic_cast<AST::IdentifierExpr *>(expr))
	{
		if (auto *sym = resolveInScope(scope, id->name, expr->line, expr->column))
		{
			return sym->typeName;
		}
		return "";
	}
	if (auto *call = dynamic_cast<AST::CallExpr *>(expr))
	{
		if (auto *sym = resolveInScope(scope, call->callee, expr->line, expr->column))
		{
			return sym->typeName;
		}
		return "";
	}
	if (auto *si = dynamic_cast<AST::StructInstanceExpr *>(expr))
	{
		return si->structName == "__anon" ? "" : si->structName;
	}
	if (auto *fa = dynamic_cast<AST::FieldAccessExpr *>(expr))
	{
		std::string objType = inferTypeName(doc, fa->object.get(), scope);
		auto        it = doc.structTypes.find(objType);
		if (it == doc.structTypes.end())
		{
			return "";
		}
		for (auto &f : it->second->fields)
		{
			if (f.name == fa->fieldName)
			{
				return f.type ? f.type->name : "";
			}
		}
		return "";
	}
	if (auto *ma = dynamic_cast<AST::MemberAccessExpr *>(expr))
	{
		std::string objType = inferTypeName(doc, ma->object.get(), scope);
		auto        it = doc.structTypes.find(objType);
		if (it == doc.structTypes.end())
		{
			return "";
		}
		for (auto &f : it->second->fields)
		{
			if (f.name == ma->member)
			{
				return f.type ? f.type->name : "";
			}
		}
		return "";
	}
	if (auto *ax = dynamic_cast<AST::ArrayAccessExpr *>(expr))
	{
		return inferTypeName(doc, ax->array.get(), scope);
	}
	if (auto *un = dynamic_cast<AST::UnaryExpr *>(expr))
	{
		return inferTypeName(doc, un->operand.get(), scope);
	}
	return "";
}

void bindType(LSP::DocumentState &doc, AST::TypeNode *t, LSP::Scope *scope)
{
	if (!t || t->line == 0)
	{
		return;
	}
	LSP::SymbolInfo *sym = nullptr;
	if (doc.structTypes.count(t->name) > 0)
	{
		sym = resolveInScope(scope, t->name, t->line, t->column);
	}
	addOccurrence(doc, t->line, t->column, t->name, t, /*isDecl=*/false, sym);
}

void bindExpr(LSP::DocumentState &doc, AST::Expression *expr, LSP::Scope *scope)
{
	if (!expr)
	{
		return;
	}

	if (auto *id = dynamic_cast<AST::IdentifierExpr *>(expr))
	{
		auto *sym = resolveInScope(scope, id->name, expr->line, expr->column);
		addOccurrence(doc, expr->line, expr->column, id->name, expr, false, sym);
		return;
	}
	if (auto *call = dynamic_cast<AST::CallExpr *>(expr))
	{
		auto *sym = resolveInScope(scope, call->callee, expr->line, expr->column);
		addOccurrence(doc, expr->line, expr->column, call->callee, expr, false, sym);
		for (auto &arg : call->arguments)
		{
			bindExpr(doc, arg.get(), scope);
		}
		return;
	}
	if (auto *fa = dynamic_cast<AST::FieldAccessExpr *>(expr))
	{
		bindExpr(doc, fa->object.get(), scope);
		std::string       objType = inferTypeName(doc, fa->object.get(), scope);
		LSP::SymbolInfo  *sym = nullptr;
		if (!objType.empty())
		{
			auto it = doc.fieldSymbols.find(objType + "." + fa->fieldName);
			if (it != doc.fieldSymbols.end())
			{
				sym = &it->second;
			}
		}
		addOccurrence(doc, expr->line, expr->column, fa->fieldName, expr, false, sym);
		return;
	}
	if (auto *ma = dynamic_cast<AST::MemberAccessExpr *>(expr))
	{
		bindExpr(doc, ma->object.get(), scope);
		std::string      objType = inferTypeName(doc, ma->object.get(), scope);
		LSP::SymbolInfo *sym = nullptr;
		if (!objType.empty())
		{
			auto it = doc.fieldSymbols.find(objType + "." + ma->member);
			if (it != doc.fieldSymbols.end())
			{
				sym = &it->second;
			}
		}
		addOccurrence(doc, expr->line, expr->column, ma->member, expr, false, sym);
		return;
	}
	if (auto *si = dynamic_cast<AST::StructInstanceExpr *>(expr))
	{
		if (si->structName != "__anon")
		{
			auto *sym = resolveInScope(scope, si->structName, expr->line, expr->column);
			addOccurrence(doc, expr->line, expr->column, si->structName, expr, false, sym);
		}
		for (auto &fv : si->fieldValues)
		{
			bindExpr(doc, fv.second.get(), scope);
		}
		return;
	}
	if (auto *bin = dynamic_cast<AST::BinaryExpr *>(expr))
	{
		bindExpr(doc, bin->left.get(), scope);
		bindExpr(doc, bin->right.get(), scope);
		return;
	}
	if (auto *un = dynamic_cast<AST::UnaryExpr *>(expr))
	{
		bindExpr(doc, un->operand.get(), scope);
		return;
	}
	if (auto *pf = dynamic_cast<AST::PostfixExpr *>(expr))
	{
		bindExpr(doc, pf->operand.get(), scope);
		return;
	}
	if (auto *asn = dynamic_cast<AST::AssignmentExpr *>(expr))
	{
		bindExpr(doc, asn->target.get(), scope);
		bindExpr(doc, asn->value.get(), scope);
		return;
	}
	if (auto *aa = dynamic_cast<AST::ArrayAccessExpr *>(expr))
	{
		bindExpr(doc, aa->array.get(), scope);
		bindExpr(doc, aa->index.get(), scope);
		return;
	}
	if (auto *al = dynamic_cast<AST::ArrayLiteralExpr *>(expr))
	{
		for (auto &e : al->elements)
		{
			bindExpr(doc, e.get(), scope);
		}
		return;
	}
}

void bindStmts(LSP::DocumentState &doc, std::vector<std::unique_ptr<AST::Statement>> &stmts, LSP::Scope *scope)
{
	for (auto &s : stmts)
	{
		bindStmt(doc, s.get(), scope);
	}
}

LSP::Scope *pushChildScope(LSP::Scope *parent, size_t startLine, size_t startCol, size_t endLine, size_t endCol)
{
	auto child = std::make_unique<LSP::Scope>();
	child->parent = parent;
	child->startLine = startLine;
	child->startColumn = startCol;
	child->endLine = endLine;
	child->endColumn = endCol;
	LSP::Scope *raw = child.get();
	parent->children.push_back(std::move(child));
	return raw;
}

void bindStmt(LSP::DocumentState &doc, AST::Statement *stmt, LSP::Scope *scope)
{
	if (!stmt)
	{
		return;
	}

	if (auto *blk = dynamic_cast<AST::BlockStmt *>(stmt))
	{
		size_t endLine = blk->endLine != 0 ? blk->endLine : scope->endLine;
		size_t endCol = blk->endColumn != 0 ? blk->endColumn : scope->endColumn;
		LSP::Scope *child = pushChildScope(scope, blk->line, blk->column, endLine, endCol);
		bindStmts(doc, blk->statements, child);
		return;
	}
	if (auto *es = dynamic_cast<AST::ExpressionStmt *>(stmt))
	{
		bindExpr(doc, es->expression.get(), scope);
		return;
	}
	if (auto *ps = dynamic_cast<AST::PrintStmt *>(stmt))
	{
		bindExpr(doc, ps->expression.get(), scope);
		return;
	}
	if (auto *vd = dynamic_cast<AST::VarDecl *>(stmt))
	{
		bindExpr(doc, vd->initializer.get(), scope);

		std::string typeName = vd->type ? vd->type->name : inferTypeName(doc, vd->initializer.get(), scope);
		std::string typeDisplay = vd->type ? formatTypeNode(vd->type.get()) : typeName;

		LSP::SymbolInfo info;
		info.name = vd->name;
		info.kind = LSP::SymbolKind::Variable;
		info.declaration = vd;
		info.typeName = typeName;
		info.typeDisplay = typeDisplay;
		info.declLine = vd->line;
		info.declColumn = vd->column;
		info.signature = "var " + vd->name + (typeDisplay.empty() ? "" : (": " + typeDisplay));

		auto *sym = declareSymbol(scope, std::move(info));
		addOccurrence(doc, vd->line, vd->column, vd->name, vd, true, sym);
		if (vd->type)
		{
			bindType(doc, vd->type.get(), scope);
		}
		return;
	}
	if (auto *rs = dynamic_cast<AST::ReturnStmt *>(stmt))
	{
		bindExpr(doc, rs->value.get(), scope);
		return;
	}
	if (auto *is = dynamic_cast<AST::IfStmt *>(stmt))
	{
		bindExpr(doc, is->condition.get(), scope);
		bindStmt(doc, is->thenBranch.get(), scope);
		bindStmt(doc, is->elseBranch.get(), scope);
		return;
	}
	if (auto *ws = dynamic_cast<AST::WhileStmt *>(stmt))
	{
		bindExpr(doc, ws->condition.get(), scope);
		bindStmt(doc, ws->body.get(), scope);
		return;
	}
	if (auto *fs = dynamic_cast<AST::ForStmt *>(stmt))
	{
		LSP::Scope *child = pushChildScope(scope, fs->line, fs->column, scope->endLine, scope->endColumn);
		bindStmt(doc, fs->initializer.get(), child);
		bindExpr(doc, fs->condition.get(), child);
		bindExpr(doc, fs->increment.get(), child);
		bindStmt(doc, fs->body.get(), child);
		return;
	}
	if (auto *sw = dynamic_cast<AST::SwitchStmt *>(stmt))
	{
		bindExpr(doc, sw->expr.get(), scope);
		LSP::Scope *child = pushChildScope(scope, sw->line, sw->column, scope->endLine, scope->endColumn);
		for (auto &c : sw->cases)
		{
			bindExpr(doc, c.value.get(), child);
			bindStmts(doc, c.statements, child);
		}
		bindStmts(doc, sw->defaultStmts, child);
		return;
	}
	if (auto *fn = dynamic_cast<AST::FunctionDecl *>(stmt))
	{
		LSP::SymbolInfo info;
		info.name = fn->name;
		info.kind = LSP::SymbolKind::Function;
		info.declaration = fn;
		info.typeName = fn->returnType ? fn->returnType->name : "";
		info.typeDisplay = fn->returnType ? formatTypeNode(fn->returnType.get()) : "void";
		info.declLine = fn->line;
		info.declColumn = fn->column;
		for (auto &p : fn->params)
		{
			info.params.emplace_back(p.name, p.type ? formatTypeNode(p.type.get()) : "");
		}
		{
			std::ostringstream sig;
			sig << "fn " << fn->name << "(";
			for (size_t i = 0; i < info.params.size(); ++i)
			{
				if (i)
				{
					sig << ", ";
				}
				sig << info.params[i].first << ": " << info.params[i].second;
			}
			sig << ")";
			if (fn->returnType)
			{
				sig << " -> " << info.typeDisplay;
			}
			info.signature = sig.str();
		}

		auto *sym = declareSymbol(scope, std::move(info));
		addOccurrence(doc, fn->line, fn->column, fn->name, fn, true, sym);
		if (fn->returnType)
		{
			bindType(doc, fn->returnType.get(), scope);
		}

		if (fn->body)
		{
			size_t endLine = fn->body->endLine != 0 ? fn->body->endLine : scope->endLine;
			size_t endCol = fn->body->endColumn != 0 ? fn->body->endColumn : scope->endColumn;
			LSP::Scope *child = pushChildScope(scope, fn->body->line, fn->body->column, endLine, endCol);

			for (auto &p : fn->params)
			{
				LSP::SymbolInfo pinfo;
				pinfo.name = p.name;
				pinfo.kind = LSP::SymbolKind::Parameter;
				pinfo.declaration = nullptr;
				pinfo.typeName = p.type ? p.type->name : "";
				pinfo.typeDisplay = p.type ? formatTypeNode(p.type.get()) : "";
				pinfo.declLine = p.line;
				pinfo.declColumn = p.column;
				pinfo.signature = p.name + (pinfo.typeDisplay.empty() ? "" : (": " + pinfo.typeDisplay));

				auto *psym = declareSymbol(child, std::move(pinfo));
				addOccurrence(doc, p.line, p.column, p.name, nullptr, true, psym);
				if (p.type)
				{
					bindType(doc, p.type.get(), child);
				}
			}

			bindStmts(doc, fn->body->statements, child);
		}
		return;
	}
	if (auto *fd = dynamic_cast<AST::ForwardDecl *>(stmt))
	{
		LSP::SymbolInfo info;
		info.name = fd->name;
		info.kind = LSP::SymbolKind::ForwardDecl;
		info.declaration = fd;
		info.typeName = fd->returnType ? fd->returnType->name : "";
		info.typeDisplay = fd->returnType ? formatTypeNode(fd->returnType.get()) : "void";
		info.declLine = fd->line;
		info.declColumn = fd->column;
		for (auto &p : fd->params)
		{
			info.params.emplace_back(p.name, p.type ? formatTypeNode(p.type.get()) : "");
		}
		std::ostringstream sig;
		sig << "fn " << fd->name << "(";
		for (size_t i = 0; i < info.params.size(); ++i)
		{
			if (i)
			{
				sig << ", ";
			}
			sig << info.params[i].first << ": " << info.params[i].second;
		}
		sig << ")";
		if (fd->returnType)
		{
			sig << " -> " << info.typeDisplay;
		}
		sig << ";  // forward declaration";
		info.signature = sig.str();

		auto *sym = declareSymbol(scope, std::move(info));
		addOccurrence(doc, fd->line, fd->column, fd->name, fd, true, sym);
		if (fd->returnType)
		{
			bindType(doc, fd->returnType.get(), scope);
		}
		return;
	}
	if (auto *st = dynamic_cast<AST::StructDecl *>(stmt))
	{
		LSP::SymbolInfo info;
		info.name = st->name;
		info.kind = LSP::SymbolKind::Struct;
		info.declaration = st;
		info.declLine = st->line;
		info.declColumn = st->column;
		{
			std::ostringstream sig;
			sig << "struct " << st->name << " { ";
			for (size_t i = 0; i < st->fields.size(); ++i)
			{
				if (i)
				{
					sig << ", ";
				}
				sig << st->fields[i].name << ": "
				    << (st->fields[i].type ? formatTypeNode(st->fields[i].type.get()) : "");
			}
			sig << " }";
			info.signature = sig.str();
		}

		auto *sym = declareSymbol(scope, std::move(info));
		addOccurrence(doc, st->line, st->column, st->name, st, true, sym);

		for (auto &f : st->fields)
		{
			LSP::SymbolInfo finfo;
			finfo.name = f.name;
			finfo.kind = LSP::SymbolKind::Field;
			finfo.declaration = st;
			finfo.typeName = f.type ? f.type->name : "";
			finfo.typeDisplay = f.type ? formatTypeNode(f.type.get()) : "";
			finfo.declLine = f.line;
			finfo.declColumn = f.column;
			finfo.signature = st->name + "." + f.name + (finfo.typeDisplay.empty() ? "" : (": " + finfo.typeDisplay));

			std::string       key = st->name + "." + f.name;
			LSP::SymbolInfo  &stored = doc.fieldSymbols[key];
			stored = std::move(finfo);
			addOccurrence(doc, f.line, f.column, f.name, nullptr, true, &stored);
			if (f.type)
			{
				bindType(doc, f.type.get(), scope);
			}
		}
		return;
	}
	if (auto *ex = dynamic_cast<AST::ExportStmt *>(stmt))
	{
		bindStmt(doc, ex->declaration.get(), scope);
		return;
	}
	if (auto *ub = dynamic_cast<AST::UnsafeBlockStmt *>(stmt))
	{
		bindStmt(doc, ub->block.get(), scope);
		return;
	}
}

void collectStructTypes(LSP::DocumentState &doc, std::vector<std::unique_ptr<AST::Statement>> &stmts)
{
	for (auto &s : stmts)
	{
		if (auto *st = dynamic_cast<AST::StructDecl *>(s.get()))
		{
			doc.structTypes[st->name] = st;
		}
		else if (auto *ex = dynamic_cast<AST::ExportStmt *>(s.get()))
		{
			if (auto *st2 = dynamic_cast<AST::StructDecl *>(ex->declaration.get()))
			{
				doc.structTypes[st2->name] = st2;
			}
		}
	}
}

} // namespace

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
	{
		return {};
	}
	return it->second.diagnostics;
}

const LSP::Occurrence *LSP::occurrenceAt(const DocumentState &doc, size_t line, size_t column) const
{
	auto byLine = [](const Occurrence &o, size_t l) { return o.line < l; };
	auto begin = std::lower_bound(doc.occurrences.begin(), doc.occurrences.end(), line, byLine);
	for (auto it = begin; it != doc.occurrences.end() && it->line == line; ++it)
	{
		if (it->contains(line, column))
		{
			return &(*it);
		}
	}
	return nullptr;
}

AST::Node *LSP::findNodeAtPosition(const std::string &uri, size_t line, size_t column)
{
	auto it = documents.find(uri);
	if (it == documents.end())
	{
		return nullptr;
	}
	const Occurrence *occ = occurrenceAt(it->second, toLexerLine(line), toLexerCol(column));
	return occ ? occ->node : nullptr;
}

std::string LSP::renderHover(const SymbolInfo &sym) const
{
	if (!sym.signature.empty())
	{
		return sym.signature;
	}
	std::string s = sym.name;
	if (!sym.typeDisplay.empty())
	{
		s += ": " + sym.typeDisplay;
	}
	return s;
}

std::optional<std::string> LSP::getHover(const std::string &uri, size_t line, size_t column)
{
	auto it = documents.find(uri);
	if (it == documents.end())
	{
		return std::nullopt;
	}
	const DocumentState &doc = it->second;
	const Occurrence     *occ = occurrenceAt(doc, toLexerLine(line), toLexerCol(column));
	if (!occ)
	{
		return std::nullopt;
	}
	if (occ->symbol)
	{
		return renderHover(*occ->symbol);
	}
	return occ->name.empty() ? std::nullopt : std::make_optional(occ->name);
}

std::optional<LSP::Location> LSP::getDefinition(const std::string &uri, size_t line, size_t column)
{
	auto it = documents.find(uri);
	if (it == documents.end())
	{
		return std::nullopt;
	}
	const DocumentState &doc = it->second;
	const Occurrence     *occ = occurrenceAt(doc, toLexerLine(line), toLexerCol(column));
	if (!occ || !occ->symbol || occ->symbol->declLine == 0)
	{
		return std::nullopt;
	}
	return Location{uri, toLspLine(occ->symbol->declLine), toLspColumn(occ->symbol->declColumn)};
}

std::vector<LSP::Location> LSP::getReferences(const std::string &uri, size_t line, size_t column,
                                               bool includeDeclaration)
{
	std::vector<Location> results;
	auto                   it = documents.find(uri);
	if (it == documents.end())
	{
		return results;
	}
	const DocumentState &doc = it->second;
	const Occurrence     *occ = occurrenceAt(doc, toLexerLine(line), toLexerCol(column));
	if (!occ || !occ->symbol)
	{
		return results;
	}
	const SymbolInfo *sym = occ->symbol;
	for (const auto &o : doc.occurrences)
	{
		if (o.symbol != sym)
		{
			continue;
		}
		if (o.isDeclaration && !includeDeclaration)
		{
			continue;
		}
		results.push_back(Location{uri, toLspLine(o.line), toLspColumn(o.startColumn)});
	}
	return results;
}

std::optional<std::vector<LSP::TextEdit>> LSP::getRenameEdits(const std::string &uri, size_t line, size_t column,
                                                                const std::string &newName)
{
	auto it = documents.find(uri);
	if (it == documents.end())
	{
		return std::nullopt;
	}
	const DocumentState &doc = it->second;
	const Occurrence     *occ = occurrenceAt(doc, toLexerLine(line), toLexerCol(column));
	if (!occ || !occ->symbol)
	{
		return std::nullopt;
	}
	const SymbolInfo      *sym = occ->symbol;
	std::vector<TextEdit>  edits;
	for (const auto &o : doc.occurrences)
	{
		if (o.symbol != sym)
		{
			continue;
		}
		edits.push_back(TextEdit{toLspLine(o.line), toLspColumn(o.startColumn), toLspColumn(o.endColumn), newName});
	}
	if (edits.empty())
	{
		return std::nullopt;
	}
	return edits;
}

std::vector<LSP::CompletionItem> LSP::getCompletions(const std::string &uri, size_t line, size_t column)
{
	std::vector<CompletionItem> results;
	auto                         it = documents.find(uri);
	if (it == documents.end() || !it->second.rootScope)
	{
		return results;
	}
	DocumentState &doc = it->second;
	size_t         lLine = toLexerLine(line);
	size_t         lCol = toLexerCol(column);

	std::string linePrefix;
	if (lLine >= 1 && lLine - 1 < doc.lineStartOffsets.size())
	{
		size_t lineStart = doc.lineStartOffsets[lLine - 1];
		size_t lineEnd = (lLine < doc.lineStartOffsets.size()) ? doc.lineStartOffsets[lLine] : doc.source.size();
		size_t cursorOffset = std::min(lineStart + (lCol > 0 ? lCol - 1 : 0), lineEnd);
		if (cursorOffset >= lineStart)
		{
			linePrefix = doc.source.substr(lineStart, cursorOffset - lineStart);
		}
	}
	std::string trimmed = linePrefix;
	while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.back())))
	{
		trimmed.pop_back();
	}

	Scope *scope = innermostScopeAt(doc.rootScope.get(), lLine, lCol);

	if (!trimmed.empty() && trimmed.back() == '.')
	{
		std::string beforeDot = trimmed.substr(0, trimmed.size() - 1);
		size_t      i = beforeDot.size();
		while (i > 0 && (std::isalnum(static_cast<unsigned char>(beforeDot[i - 1])) || beforeDot[i - 1] == '_'))
		{
			--i;
		}
		std::string ident = beforeDot.substr(i);
		if (ident.empty())
		{
			return results;
		}
		auto *sym = resolveInScope(scope, ident, lLine, lCol);
		if (!sym)
		{
			return results;
		}
		auto structIt = doc.structTypes.find(sym->typeName);
		if (structIt == doc.structTypes.end())
		{
			return results;
		}
		for (auto &f : structIt->second->fields)
		{
			CompletionItem item;
			item.label = f.name;
			item.kind = SymbolKind::Field;
			item.detail = f.type ? formatTypeNode(f.type.get()) : "";
			results.push_back(std::move(item));
		}
		return results;
	}

	std::unordered_map<std::string, bool> seen;
	for (Scope *s = scope; s != nullptr; s = s->parent)
	{
		bool isRoot = (s->parent == nullptr);
		for (auto &[name, sym] : s->symbols)
		{
			if (seen.count(name) > 0)
			{
				continue;
			}
			if (!isRoot && !lessOrEqualPos(sym.declLine, sym.declColumn, lLine, lCol))
			{
				continue;
			}
			seen[name] = true;
			CompletionItem item;
			item.label = name;
			item.kind = sym.kind;
			item.detail = !sym.signature.empty() ? sym.signature : sym.typeDisplay;
			results.push_back(std::move(item));
		}
	}

	static const std::vector<std::string> keywords = {
	    "var", "fn", "if", "else", "while", "for", "return", "true", "false", "null", "print",
	    "break", "continue", "switch", "case", "default", "include", "struct", "any",
	    "define", "undefine", "static_if", "unsafe"};
	for (const auto &kw : keywords)
	{
		if (seen.count(kw) > 0)
		{
			continue;
		}
		CompletionItem item;
		item.label = kw;
		item.isKeyword = true;
		results.push_back(std::move(item));
	}
	return results;
}

std::optional<LSP::SignatureHelpResult> LSP::getSignatureHelp(const std::string &uri, size_t line, size_t column)
{
	auto it = documents.find(uri);
	if (it == documents.end() || !it->second.rootScope)
	{
		return std::nullopt;
	}
	DocumentState &doc = it->second;
	size_t         lLine = toLexerLine(line);
	size_t         lCol = toLexerCol(column);

	if (lLine < 1 || lLine - 1 >= doc.lineStartOffsets.size())
	{
		return std::nullopt;
	}
	size_t lineStart = doc.lineStartOffsets[lLine - 1];
	size_t lineEnd = (lLine < doc.lineStartOffsets.size()) ? doc.lineStartOffsets[lLine] : doc.source.size();
	size_t cursorOffset = std::min(lineStart + (lCol > 0 ? lCol - 1 : 0), lineEnd);
	if (cursorOffset < lineStart)
	{
		return std::nullopt;
	}
	std::string linePrefix = doc.source.substr(lineStart, cursorOffset - lineStart);
	int  depth = 0;
	int  activeParam = 0;
	long openParenIdx = -1;
	for (long i = static_cast<long>(linePrefix.size()) - 1; i >= 0; --i)
	{
		char c = linePrefix[static_cast<size_t>(i)];
		if (c == ')')
		{
			++depth;
		}
		else if (c == '(')
		{
			if (depth == 0)
			{
				openParenIdx = i;
				break;
			}
			--depth;
		}
		else if (c == ',' && depth == 0)
		{
			++activeParam;
		}
	}
	if (openParenIdx < 0)
	{
		return std::nullopt;
	}

	long nameEnd = openParenIdx;
	while (nameEnd > 0 && std::isspace(static_cast<unsigned char>(linePrefix[static_cast<size_t>(nameEnd) - 1])))
	{
		--nameEnd;
	}
	long nameStart = nameEnd;
	while (nameStart > 0 && (std::isalnum(static_cast<unsigned char>(linePrefix[static_cast<size_t>(nameStart) - 1])) ||
	                         linePrefix[static_cast<size_t>(nameStart) - 1] == '_'))
	{
		--nameStart;
	}
	if (nameStart == nameEnd)
	{
		return std::nullopt;
	}
	std::string funcName = linePrefix.substr(static_cast<size_t>(nameStart), static_cast<size_t>(nameEnd - nameStart));

	Scope *scope = innermostScopeAt(doc.rootScope.get(), lLine, lCol);
	auto  *sym = resolveInScope(scope, funcName, lLine, lCol);
	if (!sym || (sym->kind != SymbolKind::Function && sym->kind != SymbolKind::ForwardDecl))
	{
		return std::nullopt;
	}

	SignatureHelpResult result;
	result.label = sym->signature;
	for (auto &[pname, ptype] : sym->params)
	{
		result.paramLabels.push_back(pname + (ptype.empty() ? "" : (": " + ptype)));
	}
	result.activeParameter = std::min(activeParam, static_cast<int>(result.paramLabels.size()) - 1);
	if (result.activeParameter < 0)
	{
		result.activeParameter = 0;
	}
	return result;
}

std::vector<LSP::DocumentSymbolInfo> LSP::getDocumentSymbols(const std::string &uri)
{
	std::vector<DocumentSymbolInfo> results;
	auto                             it = documents.find(uri);
	if (it == documents.end() || !it->second.rootScope)
	{
		return results;
	}
	const DocumentState &doc = it->second;

	for (auto &[name, sym] : doc.rootScope->symbols)
	{
		DocumentSymbolInfo info;
		info.name = sym.name;
		info.kind = sym.kind;
		info.detail = sym.signature;
		info.line = toLspLine(sym.declLine);
		info.column = toLspColumn(sym.declColumn);

		if (sym.kind == SymbolKind::Struct)
		{
			std::string prefix = sym.name + ".";
			for (auto &[key, field] : doc.fieldSymbols)
			{
				if (key.rfind(prefix, 0) != 0)
				{
					continue;
				}
				DocumentSymbolInfo fchild;
				fchild.name = field.name;
				fchild.kind = SymbolKind::Field;
				fchild.detail = field.typeDisplay;
				fchild.line = toLspLine(field.declLine);
				fchild.column = toLspColumn(field.declColumn);
				info.children.push_back(std::move(fchild));
			}
			std::sort(info.children.begin(), info.children.end(),
			          [](const DocumentSymbolInfo &a, const DocumentSymbolInfo &b) { return a.line < b.line; });
		}
		else if (sym.kind == SymbolKind::Function || sym.kind == SymbolKind::ForwardDecl)
		{
			for (auto &[pname, ptype] : sym.params)
			{
				DocumentSymbolInfo pchild;
				pchild.name = pname;
				pchild.kind = SymbolKind::Parameter;
				pchild.detail = ptype;
				pchild.line = info.line;
				pchild.column = info.column;
				info.children.push_back(std::move(pchild));
			}
		}

		results.push_back(std::move(info));
	}

	std::sort(results.begin(), results.end(), [](const DocumentSymbolInfo &a, const DocumentSymbolInfo &b) {
		if (a.line != b.line)
		{
			return a.line < b.line;
		}
		return a.column < b.column;
	});
	return results;
}

std::string LSP::symbolNameAt(AST::Node *node) const
{
	if (!node)
	{
		return "";
	}
	if (auto *e = dynamic_cast<AST::IdentifierExpr *>(node))
	{
		return e->name;
	}
	if (auto *e = dynamic_cast<AST::CallExpr *>(node))
	{
		return e->callee;
	}
	if (auto *e = dynamic_cast<AST::StructInstanceExpr *>(node))
	{
		return e->structName;
	}
	if (auto *e = dynamic_cast<AST::FieldAccessExpr *>(node))
	{
		return e->fieldName;
	}
	if (auto *e = dynamic_cast<AST::MemberAccessExpr *>(node))
	{
		return e->member;
	}
	if (auto *e = dynamic_cast<AST::FunctionDecl *>(node))
	{
		return e->name;
	}
	if (auto *e = dynamic_cast<AST::StructDecl *>(node))
	{
		return e->name;
	}
	if (auto *e = dynamic_cast<AST::VarDecl *>(node))
	{
		return e->name;
	}
	return "";
}

void LSP::computeLineOffsets(DocumentState &doc)
{
	doc.lineStartOffsets.clear();
	doc.lineStartOffsets.push_back(0);
	for (size_t i = 0; i < doc.source.size(); ++i)
	{
		if (doc.source[i] == '\n')
		{
			doc.lineStartOffsets.push_back(i + 1);
		}
	}
}

void LSP::buildIndex(DocumentState &doc)
{
	if (!doc.program)
	{
		return;
	}
	collectStructTypes(doc, doc.program->statements);
	bindStmts(doc, doc.program->statements, doc.rootScope.get());
	std::sort(doc.occurrences.begin(), doc.occurrences.end(), [](const Occurrence &a, const Occurrence &b) {
		if (a.line != b.line)
		{
			return a.line < b.line;
		}
		return a.startColumn < b.startColumn;
	});
}

void LSP::compile(DocumentState &doc)
{
	computeLineOffsets(doc);
	doc.diagnostics.clear();
	doc.program.reset();
	doc.rootScope = std::make_unique<Scope>();
	doc.rootScope->startLine = 0;
	doc.rootScope->startColumn = 0;
	doc.rootScope->endLine = SIZE_MAX;
	doc.rootScope->endColumn = SIZE_MAX;
	doc.occurrences.clear();
	doc.structTypes.clear();
	doc.fieldSymbols.clear();

	try
	{
		Lexer  lexer(doc.source);
		auto   tokens = lexer.tokenize();
		std::filesystem::path srcPath = uriToPath(doc.uri);
		bool                  haveSrcPath = !srcPath.empty() && std::filesystem::exists(srcPath);

		std::optional<Parser> parserSlot;
		if (haveSrcPath)
		{
			parserSlot.emplace(tokens, srcPath);
		}
		else
		{
			parserSlot.emplace(tokens);
		}
		Parser &parser = *parserSlot;
		parser.setIncludePaths(includePaths);
		doc.program = parser.parse(true);

		for (const auto &err : lexer.getErrors())
		{
			doc.diagnostics.push_back({err.message, toLspLine(err.line), toLspColumn(err.column), toLspLine(err.line),
			                            toLspColumn(err.column) + 1});
		}
		for (const auto &err : parser.getErrors())
		{
			doc.diagnostics.push_back({err.message, toLspLine(err.line), toLspColumn(err.column), toLspLine(err.line),
			                            toLspColumn(err.column) + 1});
		}
	}
	catch (const std::runtime_error &e)
	{
		doc.diagnostics.push_back({e.what(), 0, 0, 0, 1});
	}
	catch (...)
	{
		doc.diagnostics.push_back({"Internal error while compiling document.", 0, 0, 0, 1});
	}

	buildIndex(doc);
}

} // namespace Phasor