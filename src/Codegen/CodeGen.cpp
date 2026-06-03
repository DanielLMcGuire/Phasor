#include "CodeGen.hpp"
#include <iostream>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <phsint.hpp>

namespace Phasor
{

static void collectCallsExpr(const AST::Expression *expr, std::unordered_set<std::string> &out);
static void collectCallsStmt(const AST::Statement  *stmt, std::unordered_set<std::string> &out);

static void collectCallsExpr(const AST::Expression *expr, std::unordered_set<std::string> &out)
{
	if (!expr) return;

	if (const auto *e = dynamic_cast<const AST::CallExpr *>(expr))
	{
		out.insert(e->callee);
		for (const auto &a : e->arguments) collectCallsExpr(a.get(), out);
	}
	else if (const auto *e = dynamic_cast<const AST::BinaryExpr *>(expr))
	{
		collectCallsExpr(e->left.get(), out);
		collectCallsExpr(e->right.get(), out);
	}
	else if (const auto *e = dynamic_cast<const AST::UnaryExpr *>(expr))
	{
		collectCallsExpr(e->operand.get(), out);
	}
	else if (const auto *e = dynamic_cast<const AST::PostfixExpr *>(expr))
	{
		collectCallsExpr(e->operand.get(), out);
	}
	else if (const auto *e = dynamic_cast<const AST::AssignmentExpr *>(expr))
	{
		collectCallsExpr(e->target.get(), out);
		collectCallsExpr(e->value.get(), out);
	}
	else if (const auto *e = dynamic_cast<const AST::FieldAccessExpr *>(expr))
	{
		collectCallsExpr(e->object.get(), out);
	}
	else if (const auto *e = dynamic_cast<const AST::ArrayAccessExpr *>(expr))
	{
		collectCallsExpr(e->array.get(), out);
		collectCallsExpr(e->index.get(), out);
	}
	else if (const auto *e = dynamic_cast<const AST::ArrayLiteralExpr *>(expr))
	{
		for (const auto &elem : e->elements) collectCallsExpr(elem.get(), out);
	}
	else if (const auto *e = dynamic_cast<const AST::StructInstanceExpr *>(expr))
	{
		for (const auto &[fname, fval] : e->fieldValues) collectCallsExpr(fval.get(), out);
	}
	// Leaves (NumberExpr, StringExpr, BooleanExpr, NullExpr, IdentifierExpr) — no calls.
}

static void collectCallsStmt(const AST::Statement *stmt, std::unordered_set<std::string> &out)
{
	if (!stmt) return;

	if (const auto *s = dynamic_cast<const AST::BlockStmt *>(stmt))
	{
		for (const auto &inner : s->statements) collectCallsStmt(inner.get(), out);
	}
	else if (const auto *s = dynamic_cast<const AST::UnsafeBlockStmt *>(stmt))
	{
		collectCallsStmt(s->block.get(), out);
	}
	else if (const auto *s = dynamic_cast<const AST::ExpressionStmt *>(stmt))
	{
		collectCallsExpr(s->expression.get(), out);
	}
	else if (const auto *s = dynamic_cast<const AST::VarDecl *>(stmt))
	{
		collectCallsExpr(s->initializer.get(), out);
	}
	else if (const auto *s = dynamic_cast<const AST::PrintStmt *>(stmt))
	{
		collectCallsExpr(s->expression.get(), out);
	}
	else if (const auto *s = dynamic_cast<const AST::IfStmt *>(stmt))
	{
		collectCallsExpr(s->condition.get(), out);
		collectCallsStmt(s->thenBranch.get(), out);
		collectCallsStmt(s->elseBranch.get(), out);
	}
	else if (const auto *s = dynamic_cast<const AST::WhileStmt *>(stmt))
	{
		collectCallsExpr(s->condition.get(), out);
		collectCallsStmt(s->body.get(), out);
	}
	else if (const auto *s = dynamic_cast<const AST::ForStmt *>(stmt))
	{
		collectCallsStmt(s->initializer.get(), out);
		collectCallsExpr(s->condition.get(), out);
		collectCallsExpr(s->increment.get(), out);
		collectCallsStmt(s->body.get(), out);
	}
	else if (const auto *s = dynamic_cast<const AST::ReturnStmt *>(stmt))
	{
		collectCallsExpr(s->value.get(), out);
	}
	else if (const auto *s = dynamic_cast<const AST::SwitchStmt *>(stmt))
	{
		collectCallsExpr(s->expr.get(), out);
		for (const auto &c : s->cases)
		{
			collectCallsExpr(c.value.get(), out);
			for (const auto &cs : c.statements) collectCallsStmt(cs.get(), out);
		}
		for (const auto &ds : s->defaultStmts) collectCallsStmt(ds.get(), out);
	}
	else if (const auto *s = dynamic_cast<const AST::ExportStmt *>(stmt))
	{
		// Only scan the inner declaration if it isn't itself a function —
		// function bodies are handled via the call-graph, not by scanning
		// them as top-level code.
		if (!dynamic_cast<const AST::FunctionDecl *>(s->declaration.get()))
			collectCallsStmt(s->declaration.get(), out);
	}
	// BreakStmt, ContinueStmt, ImportStmt, IncludeStmt, StructDecl,
	// FunctionDecl (body handled separately) — nothing to traverse here.
}

Bytecode CodeGenerator::generate(const AST::Program &program, const std::unordered_map<std::string, int> &existingVars,
                                 int nextVarIdx, bool replMode)
{
	bytecode = Bytecode(); // Reset bytecode
	bytecode.variables = existingVars;
	bytecode.nextVarIndex = nextVarIdx;
	isRepl = replMode;

	declaredTypes.clear();
	inferredTypes.clear();
	inferredFieldTypes.clear();
	arrayDimensions.clear();
	arrayBaseTypes.clear();
	currentFunctionReturnType.clear();
	currentFunctionReturnDims.clear();
	currentFunctionHasReturn = false;

	liveFunctions.clear();

	if (!isRepl)
	{
		std::unordered_map<std::string, const AST::FunctionDecl *> allFunctions;
		for (const auto &stmt : program.statements)
		{
			const AST::FunctionDecl *fd = dynamic_cast<const AST::FunctionDecl *>(stmt.get());
			if (!fd)
			{
				if (const auto *es = dynamic_cast<const AST::ExportStmt *>(stmt.get()))
					fd = dynamic_cast<const AST::FunctionDecl *>(es->declaration.get());
			}
			if (fd) allFunctions[fd->name] = fd;
		}

		std::unordered_map<std::string, std::unordered_set<std::string>> callGraph;
		for (const auto &[name, fd] : allFunctions)
		{
			std::unordered_set<std::string> called;
			collectCallsStmt(fd->body.get(), called);
			callGraph[name] = std::move(called);
		}

		if (allFunctions.count("main")) liveFunctions.insert("main");

		for (const auto &stmt : program.statements)
		{
			if (const auto *es = dynamic_cast<const AST::ExportStmt *>(stmt.get()))
				if (const auto *fd = dynamic_cast<const AST::FunctionDecl *>(es->declaration.get()))
					liveFunctions.insert(fd->name);
		}

		for (const auto &[name, fd] : allFunctions)
			if (fd->keep) liveFunctions.insert(name);

		for (const auto &stmt : program.statements)
		{
			bool isFuncDecl    = (dynamic_cast<const AST::FunctionDecl *>(stmt.get()) != nullptr);
			bool isExportedFn  = false;
			if (const auto *es = dynamic_cast<const AST::ExportStmt *>(stmt.get()))
				isExportedFn = (dynamic_cast<const AST::FunctionDecl *>(es->declaration.get()) != nullptr);

			if (!isFuncDecl && !isExportedFn)
				collectCallsStmt(stmt.get(), liveFunctions);
		}

		std::queue<std::string> worklist;
		for (const auto &name : liveFunctions) worklist.push(name);

		while (!worklist.empty())
		{
			std::string fn = worklist.front();
			worklist.pop();
			auto it = callGraph.find(fn);
			if (it == callGraph.end()) continue;
			for (const auto &callee : it->second)
			{
				if (allFunctions.count(callee) && !liveFunctions.count(callee))
				{
					liveFunctions.insert(callee);
					worklist.push(callee);
				}
			}
		}

#ifdef _DEBUG
		for (const auto &[name, fd] : allFunctions)
		{
			if (!liveFunctions.count(name))
			{
				std::cerr << "\033[33m" << "WARNING: " "\033[0m" << "[UNUSED-FUNCTION] function '" << name << "' is unused and will not be emitted"
				          << " (line " << fd->line << ", column " << fd->column << ")\n";
			}
		}
#endif
	}

	for (const auto &stmt : program.statements)
	{
		generateStatement(stmt.get());
	}
	bytecode.emit(OpCode::HALT);
	return bytecode;
}

bool CodeGenerator::isLiteralExpression(const AST::Expression *expr, Value &outValue)
{
	if (const auto *numExpr = dynamic_cast<const AST::NumberExpr *>(expr))
	{
		try
		{
			if (numExpr->value.find('.') != std::string::npos)
			{
				outValue = Value(std::stod(numExpr->value));
			}
			else
			{
				outValue = Value(static_cast<i64>(std::stoll(numExpr->value)));
			}
			return true;
		}
		catch (...)
		{
			return false;
		}
	}
	if (const auto *strExpr = dynamic_cast<const AST::StringExpr *>(expr))
	{
		outValue = Value(strExpr->value);
		return true;
	}
	if (const auto *boolExpr = dynamic_cast<const AST::BooleanExpr *>(expr))
	{
		outValue = Value(boolExpr->value);
		return true;
	}
	if (dynamic_cast<const AST::NullExpr *>(expr) != nullptr)
	{
		outValue = Value();
		return true;
	}
	return false;
}

ValueType CodeGenerator::inferExpressionType(const AST::Expression *expr, bool &known)
{
	// If literal, we know the type immediately
	Value lit;
	if (isLiteralExpression(expr, lit))
	{
		known = true;
		return lit.getType();
	}

	// If identifier and we've inferred its type previously, return that
	if (const auto *ident = dynamic_cast<const AST::IdentifierExpr *>(expr))
	{
		auto it = inferredTypes.find(ident->name);
		if (it != inferredTypes.end())
		{
			known = true;
			return it->second;
		}
	}

	// Unary expression
	if (const auto *unaryExpr = dynamic_cast<const AST::UnaryExpr *>(expr))
	{
		if (unaryExpr->op == AST::UnaryOp::Not)
		{
			known = true;
			return ValueType::Bool;
		}
		if (unaryExpr->op == AST::UnaryOp::Negate)
		{
			return inferExpressionType(unaryExpr->operand.get(), known);
		}
	}

	// Binary expression
	if (const auto *binExpr = dynamic_cast<const AST::BinaryExpr *>(expr))
	{
		if (binExpr->op == AST::BinaryOp::And || binExpr->op == AST::BinaryOp::Or ||
		    binExpr->op == AST::BinaryOp::Equal || binExpr->op == AST::BinaryOp::NotEqual ||
		    binExpr->op == AST::BinaryOp::LessThan || binExpr->op == AST::BinaryOp::GreaterThan ||
		    binExpr->op == AST::BinaryOp::LessEqual || binExpr->op == AST::BinaryOp::GreaterEqual)
		{
			known = true;
			return ValueType::Bool;
		}

		bool leftKnown = false, rightKnown = false;
		ValueType leftType = inferExpressionType(binExpr->left.get(), leftKnown);
		ValueType rightType = inferExpressionType(binExpr->right.get(), rightKnown);

		if (leftKnown && rightKnown)
		{
			if (leftType == ValueType::Int && rightType == ValueType::Int)
			{
				known = true;
				return ValueType::Int;
			}
			if (leftType == ValueType::Float || rightType == ValueType::Float)
			{
				known = true;
				return ValueType::Float;
			}
			if (leftType == ValueType::String || rightType == ValueType::String)
			{
				known = true;
				return ValueType::String;
			}
		}
		else if (leftKnown)
		{
			if (leftType == ValueType::Float)
			{
				known = true;
				return ValueType::Float;
			}
		}
		else if (rightKnown)
		{
			if (rightType == ValueType::Float)
			{
				known = true;
				return ValueType::Float;
			}
		}
	}

	// Postfix expression
	if (const auto *postfixExpr = dynamic_cast<const AST::PostfixExpr *>(expr))
	{
		return inferExpressionType(postfixExpr->operand.get(), known);
	}

	// Assignment expression
	if (const auto *assignExpr = dynamic_cast<const AST::AssignmentExpr *>(expr))
	{
		return inferExpressionType(assignExpr->value.get(), known);
	}

	// Call expression
	if (const auto *callExpr = dynamic_cast<const AST::CallExpr *>(expr))
	{
		if (callExpr->callee == "len")
		{
			known = true;
			return ValueType::Int;
		}
		if (callExpr->callee == "starts_with" || callExpr->callee == "ends_with")
		{
			known = true;
			return ValueType::Bool;
		}

		auto retTypeIt = bytecode.functionReturnTypeNames.find(callExpr->callee);
		if (retTypeIt != bytecode.functionReturnTypeNames.end()) 
		{
			if (retTypeIt->second != "any") 
			{
				known = true;
				auto dimsIt = bytecode.functionReturnArrayDims.find(callExpr->callee);
				if (dimsIt != bytecode.functionReturnArrayDims.end() && !dimsIt->second.empty()) 
				{
					return ValueType::Array;
				}
				return mapTypeNameToValueType(retTypeIt->second);
			}
		}
	}

	if (dynamic_cast<const AST::ArrayLiteralExpr *>(expr))
	{
		known = true;
		return ValueType::Array;
	}

	if (dynamic_cast<const AST::StructInstanceExpr *>(expr))
	{
		known = true;
		return ValueType::Struct;
	}

	if (const auto *arrayAccess = dynamic_cast<const AST::ArrayAccessExpr *>(expr))
	{
		if (const auto *ident = dynamic_cast<const AST::IdentifierExpr *>(arrayAccess->array.get()))
		{
			auto arrayIt = arrayBaseTypes.find(ident->name);
			if (arrayIt != arrayBaseTypes.end() && arrayIt->second != "any")
			{
				known = true;
				return mapTypeNameToValueType(arrayIt->second);
			}
		}
		else if (const auto *fieldExpr = dynamic_cast<const AST::FieldAccessExpr *>(arrayAccess->array.get()))
		{
			if (const auto *objIdent = dynamic_cast<const AST::IdentifierExpr *>(fieldExpr->object.get()))
			{
				std::string structName = "";
				auto declIt = declaredTypes.find(objIdent->name);
				if (declIt != declaredTypes.end())
				{
					structName = declIt->second;
				}

				for (const auto &s : bytecode.structs)
				{
					if (!structName.empty() && s.name != structName)
						continue;

					for (size_t i = 0; i < s.fieldNames.size(); ++i)
					{
						if (s.fieldNames[i] == fieldExpr->fieldName)
						{
							known = true;
							return mapTypeNameToValueType(s.fieldTypeNames[i]);
						}
					}
				}
			}
		}
	}

	if (const auto *fieldExpr = dynamic_cast<const AST::FieldAccessExpr *>(expr))
	{
		// field on a named variable we have field-type records for
		if (const auto *objIdent = dynamic_cast<const AST::IdentifierExpr *>(fieldExpr->object.get()))
		{
			auto varIt = inferredFieldTypes.find(objIdent->name);
			if (varIt != inferredFieldTypes.end())
			{
				auto fieldIt = varIt->second.find(fieldExpr->fieldName);
				if (fieldIt != varIt->second.end())
				{
					known = true;
					return fieldIt->second;
				}
			}

			// named struct type, need to look up StructInfo
			auto typeIt = inferredTypes.find(objIdent->name);
			if (typeIt != inferredTypes.end() && typeIt->second == ValueType::Struct)
			{
				std::string structName = "";
				auto declIt = declaredTypes.find(objIdent->name);
				if (declIt != declaredTypes.end())
				{
					structName = declIt->second;
				}

				for (const auto &s : bytecode.structs)
				{
					if (!structName.empty() && s.name != structName)
						continue;

					for (size_t i = 0; i < s.fieldNames.size(); ++i)
					{
						if (s.fieldNames[i] == fieldExpr->fieldName)
						{
							known = true;
							if (!s.fieldArrayDims[i].empty())
							{
								return ValueType::Array;
							}
							return mapTypeNameToValueType(s.fieldTypeNames[i]);
						}
					}
				}
			}
		}
	}

	// Unknown
	known = false;
	return ValueType::Float; // default when unknown (not used unless known==true)
}

void CodeGenerator::generateStatement(const AST::Statement *stmt)
{
	if (const auto *varDecl = dynamic_cast<const AST::VarDecl *>(stmt))
	{
		generateVarDecl(varDecl);
	}
	else if (const auto *exprStmt = dynamic_cast<const AST::ExpressionStmt *>(stmt))
	{
		generateExpressionStmt(exprStmt);
	}
	else if (const auto *printStmt = dynamic_cast<const AST::PrintStmt *>(stmt))
	{
		generatePrintStmt(printStmt);
	}
	else if (dynamic_cast<const AST::IncludeStmt *>(stmt) != nullptr)
	{
		// preprocessor include
	}
	else if (const auto *importStmt = dynamic_cast<const AST::ImportStmt *>(stmt))
	{
		generateImportStmt(importStmt);
	}
	else if (const auto *exportStmt = dynamic_cast<const AST::ExportStmt *>(stmt))
	{
		generateExportStmt(exportStmt);
	}
	else if (const auto *blockStmt = dynamic_cast<const AST::BlockStmt *>(stmt))
	{
		generateBlockStmt(blockStmt);
	}
	else if (const auto *ifStmt = dynamic_cast<const AST::IfStmt *>(stmt))
	{
		generateIfStmt(ifStmt);
	}
	else if (const auto *whileStmt = dynamic_cast<const AST::WhileStmt *>(stmt))
	{
		generateWhileStmt(whileStmt);
	}
	else if (const auto *forStmt = dynamic_cast<const AST::ForStmt *>(stmt))
	{
		generateForStmt(forStmt);
	}
	else if (const auto *returnStmt = dynamic_cast<const AST::ReturnStmt *>(stmt))
	{
		generateReturnStmt(returnStmt);
	}
	else if (const auto *unsafeStmt = dynamic_cast<const AST::UnsafeBlockStmt *>(stmt))
	{
		generateUnsafeBlockStmt(unsafeStmt);
	}
	else if (const auto *funcDecl = dynamic_cast<const AST::FunctionDecl *>(stmt))
	{
		generateFunctionDecl(funcDecl);
	}
	else if (const auto *structDecl = dynamic_cast<const AST::StructDecl *>(stmt))
	{
		generateStructDecl(structDecl);
	}
	else if (dynamic_cast<const AST::BreakStmt *>(stmt) != nullptr)
	{
		generateBreakStmt();
	}
	else if (dynamic_cast<const AST::ContinueStmt *>(stmt) != nullptr)
	{
		generateContinueStmt();
	}
	else if (const auto *switchStmt = dynamic_cast<const AST::SwitchStmt *>(stmt))
	{
		generateSwitchStmt(switchStmt);
	}
	else
	{
		throw std::runtime_error("Unknown statement type in code generation");
	}
}

void CodeGenerator::generateExpression(const AST::Expression *expr, bool resultNeeded)
{
	if (const auto *numExpr = dynamic_cast<const AST::NumberExpr *>(expr))
	{
		generateNumberExpr(numExpr);
	}
	else if (const auto *strExpr = dynamic_cast<const AST::StringExpr *>(expr))
	{
		generateStringExpr(strExpr);
	}
	else if (const auto *identExpr = dynamic_cast<const AST::IdentifierExpr *>(expr))
	{
		generateIdentifierExpr(identExpr);
	}
	else if (const auto *unaryExpr = dynamic_cast<const AST::UnaryExpr *>(expr))
	{
		generateUnaryExpr(unaryExpr);
	}
	else if (const auto *callExpr = dynamic_cast<const AST::CallExpr *>(expr))
	{
		generateCallExpr(callExpr);
	}
	else if (const auto *binExpr = dynamic_cast<const AST::BinaryExpr *>(expr))
	{
		generateBinaryExpr(binExpr);
	}
	else if (const auto *boolExpr = dynamic_cast<const AST::BooleanExpr *>(expr))
	{
		generateBooleanExpr(boolExpr);
	}
	else if (const auto *nullExpr = dynamic_cast<const AST::NullExpr *>(expr))
	{
		generateNullExpr(nullExpr);
	}
	else if (const auto *assignExpr = dynamic_cast<const AST::AssignmentExpr *>(expr))
	{
		generateAssignmentExpr(assignExpr);
	}
	else if (const auto *structExpr = dynamic_cast<const AST::StructInstanceExpr *>(expr))
	{
		generateStructInstanceExpr(structExpr);
	}
	else if (const auto *fieldAccessExpr = dynamic_cast<const AST::FieldAccessExpr *>(expr))
	{
		generateFieldAccessExpr(fieldAccessExpr);
	}
	else if (const auto *postfixExpr = dynamic_cast<const AST::PostfixExpr *>(expr))
	{
		generatePostfixExpr(postfixExpr, resultNeeded);
	}
	else if (const auto *arrayLit = dynamic_cast<const AST::ArrayLiteralExpr *>(expr))
	{
		generateArrayLiteralExpr(arrayLit, resultNeeded);
	}
	else if (const auto *arrayAccess = dynamic_cast<const AST::ArrayAccessExpr *>(expr))
	{
		generateArrayAccessExpr(arrayAccess, resultNeeded);
	}
	else
	{
		throw std::runtime_error("Unknown expression type in code generation");
	}
}

void CodeGenerator::generateVarDecl(const AST::VarDecl *varDecl)
{
	bool hasExplicitType = (varDecl->type != nullptr);
	bool isAny = (hasExplicitType && varDecl->type->name == "any");
	bool isArrayType = (hasExplicitType && !varDecl->type->arrayDimensions.empty());
	ValueType declaredType = ValueType::Float;

	if (hasExplicitType)
	{
		declaredTypes[varDecl->name] = varDecl->type->name;

		if (isAny)
		{
			if (isArrayType)
				arrayBaseTypes[varDecl->name] = "any";

			if (varDecl->initializer)
			{
				bool known = false;
				ValueType inferred = inferExpressionType(varDecl->initializer.get(), known);
				if (known)
					inferredTypes[varDecl->name] = inferred;

				if (const auto *structExpr = dynamic_cast<const AST::StructInstanceExpr *>(varDecl->initializer.get()))
				{
					inferredTypes[varDecl->name] = ValueType::Struct;
					for (const auto &[fieldName, fieldValue] : structExpr->fieldValues)
					{
						bool fKnown = false;
						ValueType fType = inferExpressionType(fieldValue.get(), fKnown);
						if (fKnown)
							inferredFieldTypes[varDecl->name][fieldName] = fType;
					}
				}
			}
		}
		else if (isArrayType)
		{
			arrayBaseTypes[varDecl->name] = varDecl->type->name;
			arrayDimensions[varDecl->name] = varDecl->type->arrayDimensions;
			declaredType = ValueType::Array;
			inferredTypes[varDecl->name] = declaredType;
		}
		else if (bytecode.structEntries.contains(varDecl->type->name))
		{
			declaredType = ValueType::Struct;
			inferredTypes[varDecl->name] = declaredType;
		}
		else
		{
			declaredType = mapTypeNameToValueType(varDecl->type->name);
			inferredTypes[varDecl->name] = declaredType;
		}
	}

	auto declareVar = [&](const std::string &name) -> int {
		if (!scopeStack.empty())
		{
			ScopeFrame &frame = scopeStack.back();
			auto existing = bytecode.variables.find(name);
			
			if (frame.savedBindings.find(name) == frame.savedBindings.end())
			{
				frame.savedBindings[name] = (existing != bytecode.variables.end())
												? existing->second : -1;
			}

			if (frame.savedInferredTypes.find(name) == frame.savedInferredTypes.end())
			{
				auto it = inferredTypes.find(name);
				frame.savedInferredTypes[name] = (it != inferredTypes.end())
													? std::optional<ValueType>(it->second)
													: std::nullopt;
			}

			if (frame.savedArrayBaseTypes.find(name) == frame.savedArrayBaseTypes.end())
			{
				auto it = arrayBaseTypes.find(name);
				frame.savedArrayBaseTypes[name] = (it != arrayBaseTypes.end())
													? std::optional<std::string>(it->second)
													: std::nullopt;
			}

			if (frame.savedArrayDimensions.find(name) == frame.savedArrayDimensions.end())
			{
				auto it = arrayDimensions.find(name);
				frame.savedArrayDimensions[name] = (it != arrayDimensions.end())
													? std::optional<std::vector<int>>(it->second)
													: std::nullopt;
			}

			if (frame.savedDeclaredTypes.find(name) == frame.savedDeclaredTypes.end())
			{
				auto it = declaredTypes.find(name);
				frame.savedDeclaredTypes[name] = (it != declaredTypes.end())
													? std::optional<std::string>(it->second)
													: std::nullopt;
			}

			int idx = bytecode.nextVarIndex++;
			bytecode.variables[name] = idx;
			frame.declaredIndices.push_back({idx, name});
			return idx;
		}
		return bytecode.getOrCreateVar(name);
	};

	if (varDecl->initializer)
	{
		const auto *arrayLit = dynamic_cast<const AST::ArrayLiteralExpr *>(varDecl->initializer.get());

		if (hasExplicitType && !isAny)
		{
			if (!isArrayType && arrayLit)
			{
				throw std::runtime_error("Variable '" + varDecl->name +
				                         "' is declared as a scalar type but assigned an array literal.");
			}

			if (isArrayType && arrayLit)
			{
				if (!varDecl->type->arrayDimensions.empty() && varDecl->type->arrayDimensions[0] != -1)
				{
					if (arrayLit->elements.size() != static_cast<size_t>(varDecl->type->arrayDimensions[0]))
					{
						throw std::runtime_error("Array bounds error: variable '" + varDecl->name +
                                                 "' expects " + std::to_string(varDecl->type->arrayDimensions[0]) +
                                                 " elements, but got " + std::to_string(arrayLit->elements.size()));
					}
				}

				if (bytecode.structEntries.contains(varDecl->type->name))
				{
					const std::string &expectedStruct = varDecl->type->name;
					auto expectedIt = bytecode.structEntries.find(expectedStruct);
					const StructInfo &expectedInfo = bytecode.structs[expectedIt->second];

					for (size_t i = 0; i < arrayLit->elements.size(); ++i)
					{
						auto *anon = dynamic_cast<AST::StructInstanceExpr *>(arrayLit->elements[i].get());
						if (!anon || anon->structName != "__anon")
							throw std::runtime_error("Array element must be an anonymous struct literal");

						if (anon->fieldValues.size() != expectedInfo.fieldNames.size())
							throw std::runtime_error("Field count mismatch for struct '" + expectedStruct + "'");

						for (const auto &[fname, fval] : anon->fieldValues)
						{
							if (std::find(expectedInfo.fieldNames.begin(), expectedInfo.fieldNames.end(), fname) ==
							    expectedInfo.fieldNames.end())
							{
								throw std::runtime_error("Unknown field '" + fname + "' in struct '" + expectedStruct + "'");
							}
						}
					}
				}

				if (!bytecode.structEntries.contains(varDecl->type->name))
				{
					ValueType expectedElemType = mapTypeNameToValueType(varDecl->type->name);
					for (size_t i = 0; i < arrayLit->elements.size(); ++i)
					{
						bool known = false;
						ValueType elemType = inferExpressionType(arrayLit->elements[i].get(), known);
						if (known && elemType != expectedElemType)
						{
							throw std::runtime_error("Type mismatch in array initialization: element at index " +
							                         std::to_string(i) + " is not of expected type '" +
							                         varDecl->type->name + "'.");
						}
					}
				}
			}
		}

		if (const auto *binExpr = dynamic_cast<const AST::BinaryExpr *>(varDecl->initializer.get()))
		{
			generateBinaryExpr(binExpr, declaredType);
		}
		else
		{
			generateExpression(varDecl->initializer.get());
		}

		int varIndex = declareVar(varDecl->name);
		bytecode.emit(OpCode::STORE_VAR, varIndex);
	}
	else
	{
		int constIndex = bytecode.addConstant(Value());
		bytecode.emit(OpCode::PUSH_CONST, constIndex);
		int varIndex = declareVar(varDecl->name);
		bytecode.emit(OpCode::STORE_VAR, varIndex);
	}
}

void CodeGenerator::generateExpressionStmt(const AST::ExpressionStmt *exprStmt)
{
	if (isRepl)
	{
		generateExpression(exprStmt->expression.get(), true);
		bytecode.emit(OpCode::PRINT);
		return;
	}
	if (const auto *postfix = dynamic_cast<const AST::PostfixExpr *>(exprStmt->expression.get()))
	{
		generatePostfixExpr(postfix, false);
	}
	else
	{
		generateExpression(exprStmt->expression.get());
		bytecode.emit(OpCode::POP);
	}
}

void CodeGenerator::generatePrintStmt(const AST::PrintStmt *printStmt)
{
	generateExpression(printStmt->expression.get());
	bytecode.emit(OpCode::PRINT);
}

void CodeGenerator::generateImportStmt(const AST::ImportStmt *importStmt)
{
	int constIndex = bytecode.addStringConstant(importStmt->modulePath);
	bytecode.emit(OpCode::IMPORT, constIndex);
}

void CodeGenerator::generateExportStmt(const AST::ExportStmt *exportStmt)
{
	generateStatement(exportStmt->declaration.get());
}

void CodeGenerator::generateNumberExpr(const AST::NumberExpr *numExpr)
{
	try
	{
		if (numExpr->value.find('.') != std::string::npos)
		{
			f64 d = std::stod(numExpr->value);
			int constIndex = bytecode.addConstant(Value(d));
			bytecode.emit(OpCode::PUSH_CONST, constIndex);
		}
		else
		{
			i64 i = std::stoll(numExpr->value);
			int     constIndex = bytecode.addConstant(Value(i));
			bytecode.emit(OpCode::PUSH_CONST, constIndex);
		}
	}
	catch (...)
	{
		throw std::runtime_error("Invalid number format: " + numExpr->value);
	}
}

void CodeGenerator::generateStringExpr(const AST::StringExpr *strExpr)
{
	int constIndex = bytecode.addConstant(Value(strExpr->value));
	bytecode.emit(OpCode::PUSH_CONST, constIndex);
}

void CodeGenerator::generateIdentifierExpr(const AST::IdentifierExpr *identExpr)
{
	int varIndex = bytecode.getOrCreateVar(identExpr->name);
	bytecode.emit(OpCode::LOAD_VAR, varIndex);
}

void CodeGenerator::generateUnaryExpr(const AST::UnaryExpr *unaryExpr)
{
	generateExpression(unaryExpr->operand.get());

	switch (unaryExpr->op)
	{
	case AST::UnaryOp::Negate:
		bytecode.emit(OpCode::NEGATE);
		break;
	case AST::UnaryOp::Not:
		bytecode.emit(OpCode::NOT);
		break;
	case AST::UnaryOp::AddressOf:
		throw std::runtime_error("Address of not supported");
	case AST::UnaryOp::Dereference:
		throw std::runtime_error("Dereference not supported");
	}
}

void CodeGenerator::generateCallExpr(const AST::CallExpr *callExpr)
{
	if (callExpr->callee == "len" && callExpr->arguments.size() == 1)
	{
		if (const auto *strExpr = dynamic_cast<const AST::StringExpr *>(callExpr->arguments[0].get()))
		{
			auto len = (i64)strExpr->value.length();
			int  constIndex = bytecode.addConstant(Value(len));
			bytecode.emit(OpCode::PUSH_CONST, constIndex);
			return;
		}
		generateExpression(callExpr->arguments[0].get());
		bytecode.emit(OpCode::LEN);
		return;
	}

	if (callExpr->callee == "substr" && callExpr->arguments.size() == 3)
	{
		if (const auto *numExpr = dynamic_cast<const AST::NumberExpr *>(callExpr->arguments[2].get()))
		{
			if (numExpr->value == "1" || numExpr->value == "1.0")
			{
				generateExpression(callExpr->arguments[0].get());
				generateExpression(callExpr->arguments[1].get());
				bytecode.emit(OpCode::CHAR_AT);
				return;
			}
		}
	}

	if (callExpr->callee == "char_at" && callExpr->arguments.size() == 2)
	{
		generateExpression(callExpr->arguments[0].get());
		generateExpression(callExpr->arguments[1].get());
		bytecode.emit(OpCode::CHAR_AT);
		return;
	}

	if (callExpr->callee == "starts_with" && callExpr->arguments.size() == 2)
	{
		const auto *s = dynamic_cast<const AST::StringExpr *>(callExpr->arguments[0].get());
		const auto *p = dynamic_cast<const AST::StringExpr *>(callExpr->arguments[1].get());
		if ((s != nullptr) && (p != nullptr))
		{
			bool result = s->value.length() >= p->value.length() && s->value.starts_with(p->value);
			bytecode.emit(result ? OpCode::TRUE_P : OpCode::FALSE_P);
			return;
		}
	}

	if (callExpr->callee == "ends_with" && callExpr->arguments.size() == 2)
	{
		const auto *s = dynamic_cast<const AST::StringExpr *>(callExpr->arguments[0].get());
		const auto *suffix = dynamic_cast<const AST::StringExpr *>(callExpr->arguments[1].get());
		if ((s != nullptr) && (suffix != nullptr))
		{
			bool result = s->value.length() >= suffix->value.length() && s->value.ends_with(suffix->value);
			bytecode.emit(result ? OpCode::TRUE_P : OpCode::FALSE_P);
			return;
		}
	}

	for (const auto &arg : callExpr->arguments)
	{
		generateExpression(arg.get());
	}

	int constIndex = bytecode.addConstant(Value(static_cast<i64>(callExpr->arguments.size())));
	bytecode.emit(OpCode::PUSH_CONST, constIndex);

	auto entryIt = bytecode.functionEntries.find(callExpr->callee);
	if (entryIt != bytecode.functionEntries.end())
	{
		int nameIndex = bytecode.addStringConstant(callExpr->callee);
		auto itParam = bytecode.functionParamCounts.find(callExpr->callee);
		if (itParam != bytecode.functionParamCounts.end())
		{
			int expected = itParam->second;
			int got = static_cast<int>(callExpr->arguments.size());
			if (expected != got)
			{
				throw std::runtime_error("ERROR: calling function '" + callExpr->callee + "' with " + std::to_string(got)
				          + " arguments but it expects " + std::to_string(expected) + "\n");
			}
		}

		auto typeIt = bytecode.functionParamTypeNames.find(callExpr->callee);
		if (typeIt != bytecode.functionParamTypeNames.end())
		{
			const auto &paramTypes = typeIt->second;
			for (size_t i = 0; i < callExpr->arguments.size() && i < paramTypes.size(); ++i)
			{
				const std::string &expectedTypeName = paramTypes[i];
				if (expectedTypeName == "any") continue;

				bool      known   = false;
				ValueType argType = inferExpressionType(callExpr->arguments[i].get(), known);
				if (!known) continue;

				if (argType == ValueType::Array)
				{
					auto dimsIt = bytecode.functionParamArrayDims.find(callExpr->callee);
					if (dimsIt != bytecode.functionParamArrayDims.end() && i < dimsIt->second.size())
					{
						const auto &expectedDims = dimsIt->second[i];
						if (expectedDims.empty())
						{
							throw std::runtime_error("ERROR: argument " + std::to_string(i + 1) + " to '" + callExpr->callee
							          + "' expects scalar '" + expectedTypeName + "', got array (line "
							          + std::to_string(callExpr->line) + ", column " + std::to_string(callExpr->column) + ").\n");
						}
						if (expectedDims[0] != -1)
						{
							if (const auto *arrayLit = dynamic_cast<const AST::ArrayLiteralExpr *>(callExpr->arguments[i].get()))
							{
								if (arrayLit->elements.size() != static_cast<size_t>(expectedDims[0]))
								{
									throw std::runtime_error("ERROR: argument " + std::to_string(i + 1) + " to '" + callExpr->callee
									          + "' has wrong size: expected " + std::to_string(expectedDims[0])
									          + " elements, got " + std::to_string(arrayLit->elements.size())
									          + " (line " + std::to_string(callExpr->line) + ", column " + std::to_string(callExpr->column) + ").\n");
								}
							}
						}
					}
					continue;
				}
				else
				{
					auto dimsIt = bytecode.functionParamArrayDims.find(callExpr->callee);
					if (dimsIt != bytecode.functionParamArrayDims.end() && i < dimsIt->second.size())
					{
						const auto &expectedDims = dimsIt->second[i];
						if (!expectedDims.empty())
						{
							throw std::runtime_error("ERROR: argument " + std::to_string(i + 1) + " to '" + callExpr->callee
							          + "' expects array, got scalar (line "
							          + std::to_string(callExpr->line) + ", column " + std::to_string(callExpr->column) + ").\n");
						}
					}
				}

				ValueType expectedType = mapTypeNameToValueType(expectedTypeName);
				if (argType != expectedType && expectedType != ValueType::Struct)
				{
					throw std::runtime_error("ERROR: argument " + std::to_string(i + 1) + " to '" + callExpr->callee
					          + "' has wrong type: expected '" + expectedTypeName
					          + "' (line " + std::to_string(callExpr->line) + ", column " + std::to_string(callExpr->column) + ").\n");
				}
			}
		}

		bytecode.emit(OpCode::CALL, nameIndex);
	}
	else
	{
		int nameIndex = bytecode.addStringConstant(callExpr->callee);
		bytecode.emit(OpCode::CALL_NATIVE, nameIndex);
	}
}

void CodeGenerator::generateBinaryExpr(const AST::BinaryExpr *binExpr, ValueType hint)
{
	Value leftVal;
	Value rightVal;
	if (isLiteralExpression(binExpr->left.get(), leftVal) && isLiteralExpression(binExpr->right.get(), rightVal))
	{
		try
		{
			Value result;
			switch (binExpr->op)
			{
			case AST::BinaryOp::Add:
				result = leftVal + rightVal;
				break;
			case AST::BinaryOp::Subtract:
				result = leftVal - rightVal;
				break;
			case AST::BinaryOp::Multiply:
				result = leftVal * rightVal;
				break;
			case AST::BinaryOp::Divide:
				result = leftVal / rightVal;
				break;
			case AST::BinaryOp::Modulo:
				result = leftVal % rightVal;
				break;
			case AST::BinaryOp::And:
				result = leftVal.logicalAnd(rightVal);
				break;
			case AST::BinaryOp::Or:
				result = leftVal.logicalOr(rightVal);
				break;
			case AST::BinaryOp::Equal:
				result = Value(leftVal == rightVal);
				break;
			case AST::BinaryOp::NotEqual:
				result = Value(leftVal != rightVal);
				break;
			case AST::BinaryOp::LessThan:
				result = Value(leftVal < rightVal);
				break;
			case AST::BinaryOp::GreaterThan:
				result = Value(leftVal > rightVal);
				break;
			case AST::BinaryOp::LessEqual:
				result = Value(leftVal <= rightVal);
				break;
			case AST::BinaryOp::GreaterEqual:
				result = Value(leftVal >= rightVal);
				break;
			}
			if (result.isBool())
			{
				if (result.asBool())
				{
					bytecode.emit(OpCode::TRUE_P);
				}
				else
				{
					bytecode.emit(OpCode::FALSE_P);
				}
			}
			else if (result.isNull())
			{
				bytecode.emit(OpCode::NULL_VAL);
			}
			else
			{
				int constIndex = bytecode.addConstant(result);
				bytecode.emit(OpCode::PUSH_CONST, constIndex);
			}
			return;
		}
		catch (...)
		{
			std::cerr << "Unknown error in Phasor::CodeGenerator::generateBinaryExpr().\n";
		}
	}

	if (binExpr->op == AST::BinaryOp::And)
	{
		generateExpression(binExpr->left.get());
		int jumpToFalseIndex = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP_IF_FALSE, 0);
		generateExpression(binExpr->right.get());
		int jumpToEndIndex = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP, 0);
		bytecode.instructions[jumpToFalseIndex].operand1 = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::FALSE_P);
		bytecode.instructions[jumpToEndIndex].operand1 = static_cast<int>(bytecode.instructions.size());
		return;
	}
	if (binExpr->op == AST::BinaryOp::Or)
	{
		generateExpression(binExpr->left.get());
		int jumpToTrueIndex = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP_IF_TRUE, 0);
		generateExpression(binExpr->right.get());
		int jumpToEndIndex = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP, 0);
		bytecode.instructions[jumpToTrueIndex].operand1 = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::TRUE_P);
		bytecode.instructions[jumpToEndIndex].operand1 = static_cast<int>(bytecode.instructions.size());
		return;
	}

	u8 rLeft = allocateRegister();
	u8 rRight = allocateRegister();
	u8 rResult = allocateRegister();

	Value leftLiteral;
	bool  leftIsLiteral = isLiteralExpression(binExpr->left.get(), leftLiteral);
	if (leftIsLiteral)
	{
		int constIndex = bytecode.addConstant(leftLiteral);
		bytecode.emit(OpCode::LOAD_CONST_R, rLeft, constIndex);
	}
	else
	{
		generateExpression(binExpr->left.get());
		bytecode.emit(OpCode::POP_R, rLeft);
	}

	Value rightLiteral;
	bool  rightIsLiteral = isLiteralExpression(binExpr->right.get(), rightLiteral);
	if (rightIsLiteral)
	{
		int constIndex = bytecode.addConstant(rightLiteral);
		bytecode.emit(OpCode::LOAD_CONST_R, rRight, constIndex);
	}
	else
	{
		generateExpression(binExpr->right.get());
		bytecode.emit(OpCode::POP_R, rRight);
	}

	auto exprIsKnownInt = [&](const AST::Expression *e, bool isLiteral, const Value &lit) -> bool {
		if (isLiteral)
		{
			return lit.isInt();
		}
		bool known = false;
		ValueType type = inferExpressionType(e, known);
		return known && type == ValueType::Int;
	};

	bool leftKnownInt  = exprIsKnownInt(binExpr->left.get(), leftIsLiteral, leftLiteral);
	bool rightKnownInt = exprIsKnownInt(binExpr->right.get(), rightIsLiteral, rightLiteral);

	if (leftKnownInt && rightKnownInt && hint != ValueType::Float)
	{
		switch (binExpr->op)
		{
		case AST::BinaryOp::Add:
			bytecode.emit(OpCode::IADD_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::Subtract:
			bytecode.emit(OpCode::ISUB_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::Multiply:
			bytecode.emit(OpCode::IMUL_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::Divide:
			bytecode.emit(OpCode::IDIV_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::Modulo:
			bytecode.emit(OpCode::IMOD_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::And:
			bytecode.emit(OpCode::IAND_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::Or:
			bytecode.emit(OpCode::IOR_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::Equal:
			bytecode.emit(OpCode::IEQ_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::NotEqual:
			bytecode.emit(OpCode::INE_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::LessThan:
			bytecode.emit(OpCode::ILT_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::GreaterThan:
			bytecode.emit(OpCode::IGT_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::LessEqual:
			bytecode.emit(OpCode::ILE_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::GreaterEqual:
			bytecode.emit(OpCode::IGE_R, rResult, rLeft, rRight);
			break;
		}
	}
	else
	{
		switch (binExpr->op)
		{
		case AST::BinaryOp::Add:
			bytecode.emit(OpCode::FLADD_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::Subtract:
			bytecode.emit(OpCode::FLSUB_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::Multiply:
			bytecode.emit(OpCode::FLMUL_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::Divide:
			bytecode.emit(OpCode::FLDIV_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::Modulo:
			bytecode.emit(OpCode::FLMOD_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::And:
			bytecode.emit(OpCode::FLAND_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::Or:
			bytecode.emit(OpCode::FLOR_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::Equal:
			bytecode.emit(OpCode::FLEQ_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::NotEqual:
			bytecode.emit(OpCode::FLNE_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::LessThan:
			bytecode.emit(OpCode::FLLT_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::GreaterThan:
			bytecode.emit(OpCode::FLGT_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::LessEqual:
			bytecode.emit(OpCode::FLLE_R, rResult, rLeft, rRight);
			break;
		case AST::BinaryOp::GreaterEqual:
			bytecode.emit(OpCode::FLGE_R, rResult, rLeft, rRight);
			break;
		}
	}

	freeRegister(rLeft);
	freeRegister(rRight);
	bytecode.emit(OpCode::PUSH_R, rResult);
	freeRegister(rResult);
}

void CodeGenerator::generateBlockStmt(const AST::BlockStmt *blockStmt)
{
    scopeStack.push_back({});

    for (const auto &stmt : blockStmt->statements)
        generateStatement(stmt.get());

    ScopeFrame &frame = scopeStack.back();

    for (auto &[name, oldIdx] : frame.savedBindings)
    {
        if (oldIdx == -1)
            bytecode.variables.erase(name);
        else
            bytecode.variables[name] = oldIdx;
    }

    for (auto &[name, oldType] : frame.savedInferredTypes)
    {
        if (!oldType.has_value())
            inferredTypes.erase(name);
        else
            inferredTypes[name] = *oldType;
    }

    for (auto &[name, oldBase] : frame.savedArrayBaseTypes)
    {
        if (!oldBase.has_value())
            arrayBaseTypes.erase(name);
        else
            arrayBaseTypes[name] = *oldBase;
    }

    for (auto &[name, oldDims] : frame.savedArrayDimensions)
    {
        if (!oldDims.has_value())
            arrayDimensions.erase(name);
        else
            arrayDimensions[name] = *oldDims;
    }

    for (auto &[name, oldDeclType] : frame.savedDeclaredTypes)
    {
        if (!oldDeclType.has_value())
            declaredTypes.erase(name);
        else
            declaredTypes[name] = *oldDeclType;
    }

    if (!frame.declaredIndices.empty())
    {
        int scopeId = static_cast<int>(bytecode.scopeVarLists.size());
        bytecode.scopeVarLists.push_back(frame.declaredIndices);
        bytecode.emit(OpCode::EXIT_SCOPE, scopeId);
    }

    scopeStack.pop_back();
}

void CodeGenerator::generateIfStmt(const AST::IfStmt *ifStmt)
{
	generateExpression(ifStmt->condition.get());

	int jumpToElseIndex = static_cast<int>(bytecode.instructions.size());
	bytecode.emit(OpCode::JUMP_IF_FALSE, 0);

	generateStatement(ifStmt->thenBranch.get());

	if (ifStmt->elseBranch)
	{
		int jumpToEndIndex = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP, 0);

		bytecode.instructions[jumpToElseIndex].operand1 = static_cast<int>(bytecode.instructions.size());

		generateStatement(ifStmt->elseBranch.get());

		bytecode.instructions[jumpToEndIndex].operand1 = static_cast<int>(bytecode.instructions.size());
	}
	else
	{
		bytecode.instructions[jumpToElseIndex].operand1 = static_cast<int>(bytecode.instructions.size());
	}
}

void CodeGenerator::generateWhileStmt(const AST::WhileStmt *whileStmt)
{
	int loopStartIndex = static_cast<int>(bytecode.instructions.size());

	loopStartStack.push_back(loopStartIndex);
	breakJumpsStack.emplace_back();
	continueJumpsStack.emplace_back();

	generateExpression(whileStmt->condition.get());

	int jumpToEndIndex = static_cast<int>(bytecode.instructions.size());
	bytecode.emit(OpCode::JUMP_IF_FALSE, 0);

	generateStatement(whileStmt->body.get());

	for (int continueJump : continueJumpsStack.back())
	{
		bytecode.instructions[continueJump].operand1 = loopStartIndex;
	}

	bytecode.emit(OpCode::JUMP_BACK, loopStartIndex);

	int endIndex = static_cast<int>(bytecode.instructions.size());
	bytecode.instructions[jumpToEndIndex].operand1 = endIndex;
	for (int breakJump : breakJumpsStack.back())
	{
		bytecode.instructions[breakJump].operand1 = endIndex;
	}

	loopStartStack.pop_back();
	breakJumpsStack.pop_back();
	continueJumpsStack.pop_back();
}

void CodeGenerator::generateForStmt(const AST::ForStmt *forStmt)
{
	if (forStmt->initializer)
	{
		generateStatement(forStmt->initializer.get());
	}

	int loopStartIndex = static_cast<int>(bytecode.instructions.size());

	loopStartStack.push_back(loopStartIndex);
	breakJumpsStack.emplace_back();
	continueJumpsStack.emplace_back();

	int jumpToEndIndex = -1;
	if (forStmt->condition)
	{
		generateExpression(forStmt->condition.get());
		jumpToEndIndex = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP_IF_FALSE, 0);
	}

	generateStatement(forStmt->body.get());

	int incrementIndex = static_cast<int>(bytecode.instructions.size());
	for (int continueJump : continueJumpsStack.back())
	{
		bytecode.instructions[continueJump].operand1 = incrementIndex;
	}

	if (forStmt->increment)
	{
		if (const auto *postfix = dynamic_cast<const AST::PostfixExpr *>(forStmt->increment.get()))
		{
			generatePostfixExpr(postfix, false);
		}
		else
		{
			generateExpression(forStmt->increment.get());
			bytecode.emit(OpCode::POP);
		}
	}

	bytecode.emit(OpCode::JUMP_BACK, loopStartIndex);

	int endIndex = static_cast<int>(bytecode.instructions.size());
	if (jumpToEndIndex != -1)
	{
		bytecode.instructions[jumpToEndIndex].operand1 = endIndex;
	}
	for (int breakJump : breakJumpsStack.back())
	{
		bytecode.instructions[breakJump].operand1 = endIndex;
	}

	loopStartStack.pop_back();
	breakJumpsStack.pop_back();
	continueJumpsStack.pop_back();
}

void CodeGenerator::generateBreakStmt()
{
	if (breakJumpsStack.empty())
	{
		throw std::runtime_error("'break' statement outside of loop");
	}
	int jumpIndex = static_cast<int>(bytecode.instructions.size());
	bytecode.emit(OpCode::JUMP, 0);
	breakJumpsStack.back().push_back(jumpIndex);
}

void CodeGenerator::generateContinueStmt()
{
	if (continueJumpsStack.empty())
	{
		throw std::runtime_error("'continue' statement outside of loop");
	}
	int jumpIndex = static_cast<int>(bytecode.instructions.size());
	bytecode.emit(OpCode::JUMP, 0);
	continueJumpsStack.back().push_back(jumpIndex);
}

void CodeGenerator::generateReturnStmt(const AST::ReturnStmt *returnStmt)
{
	currentFunctionHasReturn = true;

	if (returnStmt->value)
	{
		if (!currentFunctionReturnType.empty() &&
		    currentFunctionReturnType != "any"  &&
		    currentFunctionReturnType != "void")
		{
			bool      known   = false;
			ValueType retType = inferExpressionType(returnStmt->value.get(), known);
			if (known)
			{
				bool expectedIsArray = !currentFunctionReturnDims.empty();
				if (retType == ValueType::Array && !expectedIsArray)
				{
					throw std::runtime_error("ERROR: return type mismatch: function declared to return scalar '"
					          + currentFunctionReturnType + "' but returning array"
					          + " (line " + std::to_string(returnStmt->line) + ", column " + std::to_string(returnStmt->column) + ").\n");
				}
				else if (retType != ValueType::Array && expectedIsArray)
				{
					throw std::runtime_error("ERROR: return type mismatch: function declared to return array '"
					          + currentFunctionReturnType + "[]' but returning scalar"
					          + " (line " + std::to_string(returnStmt->line) + ", column " + std::to_string(returnStmt->column) + ").\n");
				}
				else if (!expectedIsArray)
				{
					ValueType expectedType = mapTypeNameToValueType(currentFunctionReturnType);
					if (retType != expectedType && expectedType != ValueType::Struct)
					{
						throw std::runtime_error("ERROR: return type mismatch: function declared to return '"
						          + currentFunctionReturnType + "' but returning a different type"
						          + " (line " + std::to_string(returnStmt->line) + ", column " + std::to_string(returnStmt->column) + ").\n");
					}
				}
			}
		}

		generateExpression(returnStmt->value.get());
	}
	else
	{
		if (!currentFunctionReturnType.empty() &&
		    currentFunctionReturnType != "any"  &&
		    currentFunctionReturnType != "void")
		{
			throw std::runtime_error("ERROR: bare 'return' in function declared to return '"
			          + currentFunctionReturnType + "'"
			          + " (line " + std::to_string(returnStmt->line) + ", column " + std::to_string(returnStmt->column) + ").\n");
		}
		bytecode.emit(OpCode::NULL_VAL);
	}
	bytecode.emit(OpCode::RETURN);
}

void CodeGenerator::generateUnsafeBlockStmt(const AST::UnsafeBlockStmt *unsafeStmt)
{
	generateBlockStmt(unsafeStmt->block.get());
}

void CodeGenerator::generateFunctionDecl(const AST::FunctionDecl *funcDecl)
{
	// DCE: skip functions that were determined to be unreachable.
	// In REPL mode liveFunctions is always empty and this check is bypassed.
	if (!isRepl && !liveFunctions.count(funcDecl->name))
		return;

	int jumpOverIndex = static_cast<int>(bytecode.instructions.size());
	bytecode.emit(OpCode::JUMP, 0);

	int entryPoint = static_cast<int>(bytecode.instructions.size());
	bytecode.functionEntries[funcDecl->name] = entryPoint;

	bytecode.functionParamCounts[funcDecl->name] = static_cast<int>(funcDecl->params.size());

	std::vector<std::string> paramTypeNames;
	std::vector<std::vector<int>> paramArrayDims; 
	paramTypeNames.reserve(funcDecl->params.size());
	paramArrayDims.reserve(funcDecl->params.size()); 

	for (const auto &p : funcDecl->params)
	{
		paramTypeNames.push_back(p.type ? p.type->name : "any");
		paramArrayDims.push_back(p.type ? p.type->arrayDimensions : std::vector<int>{}); 
	}
	bytecode.functionParamTypeNames[funcDecl->name] = std::move(paramTypeNames);
	bytecode.functionParamArrayDims[funcDecl->name] = std::move(paramArrayDims); 

	std::string prevReturnType = currentFunctionReturnType;
	std::vector<int> prevReturnDims = currentFunctionReturnDims;
	bool prevHasReturn = currentFunctionHasReturn;

	currentFunctionReturnType = (funcDecl->returnType ? funcDecl->returnType->name : "any");
	currentFunctionReturnDims = (funcDecl->returnType ? funcDecl->returnType->arrayDimensions : std::vector<int>{});
	currentFunctionHasReturn = false;

	bytecode.functionReturnTypeNames[funcDecl->name] = currentFunctionReturnType;
	bytecode.functionReturnArrayDims[funcDecl->name] = currentFunctionReturnDims;
	bytecode.emit(OpCode::POP);

	for (auto it = funcDecl->params.rbegin(); it != funcDecl->params.rend(); ++it)
	{
		int varIndex = bytecode.getOrCreateVar(it->name);
		bytecode.emit(OpCode::STORE_VAR, varIndex);

		if (it->type && it->type->name != "any")
		{
			bool isArrayParam = !it->type->arrayDimensions.empty();
			if (isArrayParam)
			{
				arrayBaseTypes[it->name] = it->type->name;
				arrayDimensions[it->name] = it->type->arrayDimensions; 
				inferredTypes[it->name]  = ValueType::Array;
			}
			else
			{
				inferredTypes[it->name] = mapTypeNameToValueType(it->type->name);
			}
		}
	}

	generateBlockStmt(funcDecl->body.get());

	if (!currentFunctionHasReturn)
	{
		if (!currentFunctionReturnType.empty() &&
		    currentFunctionReturnType != "any"  &&
		    currentFunctionReturnType != "void")
		{
			throw std::runtime_error("ERROR: function '" + funcDecl->name + "' is declared to return '"
			          + currentFunctionReturnType + "' but has no return statement.\n");
		}
	}

	if (bytecode.instructions.empty() || bytecode.instructions.back().op != OpCode::RETURN)
	{
		bytecode.emit(OpCode::NULL_VAL);
		bytecode.emit(OpCode::RETURN);
	}

	currentFunctionReturnType = prevReturnType;
	currentFunctionReturnDims = prevReturnDims;
	currentFunctionHasReturn = prevHasReturn;

	bytecode.instructions[jumpOverIndex].operand1 = static_cast<int>(bytecode.instructions.size());
}

void CodeGenerator::generateBooleanExpr(const AST::BooleanExpr *boolExpr)
{
	if (boolExpr->value)
	{
		bytecode.emit(OpCode::TRUE_P);
	}
	else
	{
		bytecode.emit(OpCode::FALSE_P);
	}
}

void CodeGenerator::generateNullExpr(const AST::NullExpr *)
{
	bytecode.emit(OpCode::NULL_VAL);
}

void CodeGenerator::generateAssignmentExpr(const AST::AssignmentExpr *assignExpr)
{
    if (const auto *identExpr = dynamic_cast<const AST::IdentifierExpr *>(assignExpr->target.get()))
    {
        auto declIt = declaredTypes.find(identExpr->name);
        bool isAny = (declIt == declaredTypes.end() || declIt->second == "any");

        auto arrayIt = arrayBaseTypes.find(identExpr->name);
        bool isArrayTarget = (arrayIt != arrayBaseTypes.end());

        bool knownValType = false;
        ValueType valType = inferExpressionType(assignExpr->value.get(), knownValType);

        if (!isAny)
        {
            if (isArrayTarget)
            {
                if (knownValType && valType != ValueType::Array) {
                    throw std::runtime_error("Type mismatch: cannot assign non-array to array variable '" + identExpr->name + "'");
                }
                
                std::string expectedBaseType = arrayIt->second;
                if (expectedBaseType != "any")
                {
                    ValueType expectedElemType = mapTypeNameToValueType(expectedBaseType);
                    if (const auto *arrayLit = dynamic_cast<const AST::ArrayLiteralExpr *>(assignExpr->value.get()))
                    {
                        auto dimsIt = arrayDimensions.find(identExpr->name);
                        if (dimsIt != arrayDimensions.end() && !dimsIt->second.empty() && dimsIt->second[0] != -1)
                        {
                            if (arrayLit->elements.size() != static_cast<size_t>(dimsIt->second[0]))
                                throw std::runtime_error("Array bounds error: assigning " + std::to_string(arrayLit->elements.size()) +
                                                         " elements to array '" + identExpr->name + "' of size " + std::to_string(dimsIt->second[0]));
                        }

                        for (size_t i = 0; i < arrayLit->elements.size(); ++i)
                        {
                            bool elemKnown = false;
                            ValueType elemType = inferExpressionType(arrayLit->elements[i].get(), elemKnown);
                            if (elemKnown && elemType != expectedElemType)
                            {
                                throw std::runtime_error("Type mismatch in array assignment: element at index " +
                                                         std::to_string(i) + " does not match expected base type '" +
                                                         expectedBaseType + "'.");
                            }
                        }
                    }
                }
            }
            else
            {
                if (knownValType && valType == ValueType::Array) {
                    throw std::runtime_error("Type mismatch: cannot assign array to scalar variable '" + identExpr->name + "'");
                }
                
                ValueType expectedType = mapTypeNameToValueType(declIt->second);
                if (knownValType && valType != expectedType && expectedType != ValueType::Null && expectedType != ValueType::Struct)
                {
                    throw std::runtime_error("Type mismatch: cannot assign value of mismatching type to variable '" + identExpr->name + "' of type '" + declIt->second + "'");
                }
            }
        }

        ValueType targetType = ValueType::Null;
        auto typeIt = inferredTypes.find(identExpr->name);
        if (typeIt != inferredTypes.end())
            targetType = typeIt->second;

        if (const auto *binExpr = dynamic_cast<const AST::BinaryExpr *>(assignExpr->value.get()))
        {
            generateBinaryExpr(binExpr, targetType);
        }
        else
        {
            generateExpression(assignExpr->value.get());
        }

        Value val;
        if (isLiteralExpression(assignExpr->value.get(), val))
        {
            inferredTypes[identExpr->name] = val.getType();
        }

        int varIndex = bytecode.getOrCreateVar(identExpr->name);
        bytecode.emit(OpCode::STORE_VAR, varIndex);
        bytecode.emit(OpCode::LOAD_VAR, varIndex);
    }
    else if (const auto *fieldExpr = dynamic_cast<const AST::FieldAccessExpr *>(assignExpr->target.get()))
    {
        generateExpression(fieldExpr->object.get());
        generateExpression(assignExpr->value.get());

        std::string tempName = "__assign_val_" + std::to_string(switchCounter++);
        int tempVar = bytecode.getOrCreateVar(tempName);
        bytecode.emit(OpCode::STORE_VAR, tempVar); 
        bytecode.emit(OpCode::LOAD_VAR, tempVar);  

        int fieldNameIndex = bytecode.addStringConstant(fieldExpr->fieldName);
        bytecode.emit(OpCode::SET_FIELD, fieldNameIndex);
        bytecode.emit(OpCode::POP); 

        bytecode.emit(OpCode::LOAD_VAR, tempVar); 
    }
    else if (const auto *arrayAccess = dynamic_cast<const AST::ArrayAccessExpr *>(assignExpr->target.get()))
    {
        if (const auto *identExpr = dynamic_cast<const AST::IdentifierExpr *>(arrayAccess->array.get()))
        {
            auto dimsIt = arrayDimensions.find(identExpr->name);
            if (dimsIt != arrayDimensions.end() && !dimsIt->second.empty() && dimsIt->second[0] != -1)
            {
                Value idxVal;
                if (isLiteralExpression(arrayAccess->index.get(), idxVal) && idxVal.isInt())
                {
                    i64 idx = idxVal.asInt();
                    if (idx < 0 || idx >= dimsIt->second[0])
                    {
                        throw std::runtime_error("Compile-time bounds error: index " + std::to_string(idx) +
                                                 " is out of bounds for array '" + identExpr->name +
                                                 "' of size " + std::to_string(dimsIt->second[0]));
                    }
                }
            }

            auto arrayIt = arrayBaseTypes.find(identExpr->name);
            if (arrayIt != arrayBaseTypes.end())
            {
                std::string expectedBaseType = arrayIt->second;
                if (expectedBaseType != "any")
                {
                    ValueType expectedElemType = mapTypeNameToValueType(expectedBaseType);
                    bool      known = false;
                    ValueType valType = inferExpressionType(assignExpr->value.get(), known);
                    if (known && valType != expectedElemType)
                    {
                        throw std::runtime_error("Type mismatch: cannot assign value of mismatching type to array element of type '" +
                                                 expectedBaseType + "'.");
                    }
                }
            }
        }

        generateExpression(arrayAccess->array.get());
        generateExpression(arrayAccess->index.get());
        generateExpression(assignExpr->value.get());

        int countIdx = bytecode.addConstant(Value(static_cast<i64>(3)));
        bytecode.emit(OpCode::PUSH_CONST, countIdx);

        int setIdx = bytecode.addStringConstant("__set_elem");
        bytecode.emit(OpCode::CALL_NATIVE, setIdx);
    }
    else
    {
        throw std::runtime_error("Invalid assignment target.");
    }
}

void CodeGenerator::generateStructInstanceExpr(const AST::StructInstanceExpr *expr)
{
	auto it = bytecode.structEntries.find(expr->structName);
	if (it != bytecode.structEntries.end())
	{
		int structIndex = it->second;

		bytecode.emit(OpCode::NEW_STRUCT_INSTANCE_STATIC, structIndex);

		for (const auto &[fieldName, fieldValue] : expr->fieldValues)
		{
			generateExpression(fieldValue.get());
			int fieldNameIndex = bytecode.addStringConstant(fieldName);
			bytecode.emit(OpCode::SET_FIELD, fieldNameIndex);
		}
	}
	else
	{
		int structNameIndex = bytecode.addStringConstant(expr->structName);
		bytecode.emit(OpCode::NEW_STRUCT, structNameIndex);

		for (const auto &[fieldName, fieldValue] : expr->fieldValues)
		{
			generateExpression(fieldValue.get());
			int fieldNameIndex = bytecode.addStringConstant(fieldName);
			bytecode.emit(OpCode::SET_FIELD, fieldNameIndex);
		}
	}
}

void CodeGenerator::generateFieldAccessExpr(const AST::FieldAccessExpr *expr)
{
	generateExpression(expr->object.get());
	int fieldNameIndex = bytecode.addStringConstant(expr->fieldName);
	bytecode.emit(OpCode::GET_FIELD, fieldNameIndex);
}

void CodeGenerator::generatePostfixExpr(const AST::PostfixExpr *expr, bool resultNeeded)
{
    const auto *identExpr = dynamic_cast<const AST::IdentifierExpr *>(expr->operand.get());
    if (identExpr == nullptr)
        throw std::runtime_error("Postfix operators only supported on variables");

    int varIndex = bytecode.getOrCreateVar(identExpr->name);

    if (resultNeeded)
        bytecode.emit(OpCode::LOAD_VAR, varIndex);

    bytecode.emit(OpCode::LOAD_VAR, varIndex);

    int oneIndex = bytecode.addConstant(Value(static_cast<i64>(1)));
    bytecode.emit(OpCode::PUSH_CONST, oneIndex);

    auto it = inferredTypes.find(identExpr->name);
    bool varIsInt = (it != inferredTypes.end() && it->second == ValueType::Int);
    if (expr->op == AST::PostfixOp::Increment)
        bytecode.emit(varIsInt ? OpCode::IADD : OpCode::FLADD);
    else
        bytecode.emit(varIsInt ? OpCode::ISUBTRACT : OpCode::FLSUBTRACT);

    bytecode.emit(OpCode::STORE_VAR, varIndex);
}

void CodeGenerator::generateStructDecl(const AST::StructDecl *decl)
{
	std::string structDef = "struct " + decl->name + " {";
	for (const auto &field : decl->fields)
	{
		structDef += " " + field.name + ":" + field.type->name + ",";
	}
	if (!decl->fields.empty())
	{
		structDef.pop_back();
	}
	structDef += " }";
	bytecode.addConstant(Value(structDef));

	int firstConstIndex = static_cast<int>(bytecode.constants.size());
	for (const auto &field : decl->fields)
	{
		(void)field;
		bytecode.addConstant(Value());
	}

	StructInfo info;
	info.name = decl->name;
	info.firstConstIndex = firstConstIndex;
	info.fieldCount = static_cast<int>(decl->fields.size());
	for (const auto &field : decl->fields)
	{
		info.fieldNames.push_back(field.name);
		info.fieldArrayDims.push_back(field.type->arrayDimensions);
		info.fieldTypeNames.push_back(field.type->name);
	}

	if (!bytecode.structEntries.contains(decl->name))
	{
		int index = static_cast<int>(bytecode.structs.size());
		bytecode.structs.push_back(std::move(info));
		bytecode.structEntries[decl->name] = index;
	}
}

void CodeGenerator::generateSwitchStmt(const AST::SwitchStmt *switchStmt)
{
	generateExpression(switchStmt->expr.get());
	std::string tempName = "__switch_" + std::to_string(switchCounter++);
	int         tempVarIndex = bytecode.getOrCreateVar(tempName);
	bytecode.emit(OpCode::STORE_VAR, tempVarIndex);

	bool known = false;
	ValueType inferred = inferExpressionType(switchStmt->expr.get(), known);
	if (known)
	{
		inferredTypes[tempName] = inferred;
	}

	std::vector<int> endJumps; 

	for (const auto &caseClause : switchStmt->cases)
	{
		bytecode.emit(OpCode::LOAD_VAR, tempVarIndex);
		generateExpression(caseClause.value.get());
		bytecode.emit(OpCode::FLEQUAL);

		int skipJump = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP_IF_FALSE, 0); 

		for (const auto &stmt : caseClause.statements)
		{
			generateStatement(stmt.get());
		}

		int endJump = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP, 0); 
		endJumps.push_back(endJump);

		bytecode.instructions[skipJump].operand1 = static_cast<int>(bytecode.instructions.size());
	}

	for (const auto &stmt : switchStmt->defaultStmts)
	{
		generateStatement(stmt.get());
	}

	int endIndex = static_cast<int>(bytecode.instructions.size());
	for (int jumpIdx : endJumps)
	{
		bytecode.instructions[jumpIdx].operand1 = endIndex;
	}
}

void CodeGenerator::generateArrayLiteralExpr(const AST::ArrayLiteralExpr *arrayLit, bool resultNeeded)
{
    for (const auto &elem : arrayLit->elements)
        generateExpression(elem.get());

    int count = static_cast<int>(arrayLit->elements.size());
    int countIdx = bytecode.addConstant(Value(static_cast<i64>(count)));
    bytecode.emit(OpCode::PUSH_CONST, countIdx);

    int funcNameIdx = bytecode.addStringConstant("__array_literal");
    bytecode.emit(OpCode::CALL_NATIVE, funcNameIdx);

    if (!resultNeeded)
        bytecode.emit(OpCode::POP);
}

void CodeGenerator::generateArrayAccessExpr(const AST::ArrayAccessExpr *arrayAccess, bool resultNeeded)
{
    if (const auto *identExpr = dynamic_cast<const AST::IdentifierExpr *>(arrayAccess->array.get())) {
        auto dimsIt = arrayDimensions.find(identExpr->name);
        if (dimsIt != arrayDimensions.end() && !dimsIt->second.empty() && dimsIt->second[0] != -1) {
            Value idxVal;
            if (isLiteralExpression(arrayAccess->index.get(), idxVal) && idxVal.isInt()) {
                i64 idx = idxVal.asInt();
                if (idx < 0 || idx >= dimsIt->second[0]) {
                    throw std::runtime_error("Compile-time bounds error: index " + std::to_string(idx) + 
                                             " is out of bounds for array '" + identExpr->name + 
                                             "' of size " + std::to_string(dimsIt->second[0]));
                }
            }
        }
    }
    else if (const auto *fieldExpr = dynamic_cast<const AST::FieldAccessExpr *>(arrayAccess->array.get()))
    {
        if (const auto *objIdent = dynamic_cast<const AST::IdentifierExpr *>(fieldExpr->object.get()))
        {
            std::string structName = "";
            auto declIt = declaredTypes.find(objIdent->name);
            if (declIt != declaredTypes.end())
            {
                structName = declIt->second;
            }

            for (const auto &s : bytecode.structs)
            {
                if (!structName.empty() && s.name != structName)
                    continue;

                for (size_t i = 0; i < s.fieldNames.size(); ++i)
                {
                    if (s.fieldNames[i] == fieldExpr->fieldName)
                    {
                        const auto &dims = s.fieldArrayDims[i];
                        if (!dims.empty() && dims[0] != -1)
                        {
                            Value idxVal;
                            if (isLiteralExpression(arrayAccess->index.get(), idxVal) && idxVal.isInt())
                            {
                                i64 idx = idxVal.asInt();
                                if (idx < 0 || idx >= dims[0])
                                {
                                    throw std::runtime_error("Compile-time bounds error: index " + std::to_string(idx) + 
                                                             " is out of bounds for field array '" + fieldExpr->fieldName + 
                                                             "' of size " + std::to_string(dims[0]));
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    generateExpression(arrayAccess->array.get());   
    generateExpression(arrayAccess->index.get());   
    int countIdx = bytecode.addConstant(Value(static_cast<i64>(2)));
    bytecode.emit(OpCode::PUSH_CONST, countIdx);    
    int funcIdx = bytecode.addStringConstant("__get_elem");
    bytecode.emit(OpCode::CALL_NATIVE, funcIdx);
    if (!resultNeeded)
        bytecode.emit(OpCode::POP);
}

ValueType CodeGenerator::mapTypeNameToValueType(const std::string &typeName)
{
    if (typeName == "int" || typeName == "i64")   return ValueType::Int;
    if (typeName == "float" || typeName == "f64") return ValueType::Float;
    if (typeName == "string")                     return ValueType::String;
    if (typeName == "bool")                       return ValueType::Bool;
    if (typeName == "struct") [[unlikely]]        return ValueType::Struct;
    if (bytecode.structEntries.find(typeName) != bytecode.structEntries.end())
    { [[likely]] return ValueType::Struct; }
    if (typeName == "void")                       return ValueType::Null;

    return ValueType::Float;
}

} // namespace Phasor