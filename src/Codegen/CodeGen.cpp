#include "CodeGen.hpp"
#include <iostream>
#include <unordered_map>
#include <phsint.hpp>

namespace Phasor
{

Bytecode CodeGenerator::generate(const AST::Program &program, const std::unordered_map<std::string, int> &existingVars,
                                 int nextVarIdx, bool replMode)
{
	bytecode = Bytecode(); // Reset bytecode
	bytecode.variables = existingVars;
	bytecode.nextVarIndex = nextVarIdx;
	isRepl = replMode;

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
		if (isAny)
		{
			// 'any' opts out of all type tracking — leave inferredTypes unpopulated
			if (isArrayType)
				arrayBaseTypes[varDecl->name] = "any";
		}
		else if (isArrayType)
		{
			arrayBaseTypes[varDecl->name] = varDecl->type->name;
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

	if (varDecl->initializer)
	{
		const auto *arrayLit = dynamic_cast<const AST::ArrayLiteralExpr *>(varDecl->initializer.get());

		if (hasExplicitType && !isAny)
		{
			// Only literal arrays can be element checked here.
			// Non literal array values like state.board are allowed through.
			if (!isArrayType && arrayLit)
			{
				throw std::runtime_error("Variable '" + varDecl->name +
				                         "' is declared as a scalar type but assigned an array literal.");
			}

			if (isArrayType && arrayLit)
			{
				// For typed struct arrays, validate element field names/count
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

				// For typed primitive arrays, validate element types
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

		generateExpression(varDecl->initializer.get());
		int varIndex = bytecode.getOrCreateVar(varDecl->name);
		bytecode.emit(OpCode::STORE_VAR, varIndex);
	}
	else
	{
		int constIndex = bytecode.addConstant(Value());
		bytecode.emit(OpCode::PUSH_CONST, constIndex);
		int varIndex = bytecode.getOrCreateVar(varDecl->name);
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
	// For now, export just executes the declaration in the current scope
	generateStatement(exportStmt->declaration.get());
}

void CodeGenerator::generateNumberExpr(const AST::NumberExpr *numExpr)
{
	// Try to parse as integer first, then as float
	try
	{
		// Check if it has a decimal point
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
	// Generate operand
	generateExpression(unaryExpr->operand.get());

	// Emit operation
	switch (unaryExpr->op)
	{
	case AST::UnaryOp::Negate:
		bytecode.emit(OpCode::NEGATE);
		break;
	case AST::UnaryOp::Not:
		bytecode.emit(OpCode::NOT);
		break;
	case AST::UnaryOp::AddressOf:
		// Not implemented
		//	bytecode.emit(OpCode::ADROF);
		[[fallthrough]]; // for now
	case AST::UnaryOp::Dereference:
		//	bytecode.emit(OpCode::DREF);
		break;
	}
}

void CodeGenerator::generateCallExpr(const AST::CallExpr *callExpr)
{
	// Optimizations
	if (callExpr->callee == "len" && callExpr->arguments.size() == 1)
	{
		if (const auto *strExpr = dynamic_cast<const AST::StringExpr *>(callExpr->arguments[0].get()))
		{
			// Constant fold len("literal")
			auto len = (i64)strExpr->value.length();
			int  constIndex = bytecode.addConstant(Value(len));
			bytecode.emit(OpCode::PUSH_CONST, constIndex);
			return;
		}
		// Emit specialized opcode for variable strings
		generateExpression(callExpr->arguments[0].get());
		bytecode.emit(OpCode::LEN);
		return;
	}

	if (callExpr->callee == "substr" && callExpr->arguments.size() == 3)
	{
		// Check for substr(s, i, 1) -> char_at(s, i)
		if (const auto *numExpr = dynamic_cast<const AST::NumberExpr *>(callExpr->arguments[2].get()))
		{
			if (numExpr->value == "1" || numExpr->value == "1.0")
			{
				// Redirect to char_at opcode
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
	// Push arguments
	for (const auto &arg : callExpr->arguments)
	{
		generateExpression(arg.get());
	}

	// Push argument count
	int constIndex = bytecode.addConstant(Value(static_cast<i64>(callExpr->arguments.size())));
	bytecode.emit(OpCode::PUSH_CONST, constIndex);

	// Check if it's a user function
	auto entryIt = bytecode.functionEntries.find(callExpr->callee);
	if (entryIt != bytecode.functionEntries.end())
	{
		int  nameIndex = bytecode.addStringConstant(callExpr->callee);
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

				if (argType == ValueType::Array) continue;

				ValueType expectedType = mapTypeNameToValueType(expectedTypeName);
				if (argType != expectedType)
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

void CodeGenerator::generateBinaryExpr(const AST::BinaryExpr *binExpr)
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

	// Reuse literal detection done here to choose appropriate register opcodes.
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

	bool leftKnownInt = exprIsKnownInt(binExpr->left.get(), leftIsLiteral, leftLiteral);
	bool rightKnownInt = exprIsKnownInt(binExpr->right.get(), rightIsLiteral, rightLiteral);

	// Choose integer ops only when both operands are known ints.
	if (leftKnownInt && rightKnownInt)
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
		// Fallback to float ops when we don't know both operands are integers.
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
	for (const auto &stmt : blockStmt->statements)
	{
		generateStatement(stmt.get());
	}
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

	// Push loop context
	loopStartStack.push_back(loopStartIndex);
	breakJumpsStack.emplace_back();
	continueJumpsStack.emplace_back();

	generateExpression(whileStmt->condition.get());

	int jumpToEndIndex = static_cast<int>(bytecode.instructions.size());
	bytecode.emit(OpCode::JUMP_IF_FALSE, 0);

	generateStatement(whileStmt->body.get());

	// Patch continue jumps to loop start
	for (int continueJump : continueJumpsStack.back())
	{
		bytecode.instructions[continueJump].operand1 = loopStartIndex;
	}

	bytecode.emit(OpCode::JUMP_BACK, loopStartIndex);

	// Patch jump to end and break jumps
	int endIndex = static_cast<int>(bytecode.instructions.size());
	bytecode.instructions[jumpToEndIndex].operand1 = endIndex;
	for (int breakJump : breakJumpsStack.back())
	{
		bytecode.instructions[breakJump].operand1 = endIndex;
	}

	// Pop loop context
	loopStartStack.pop_back();
	breakJumpsStack.pop_back();
	continueJumpsStack.pop_back();
}

void CodeGenerator::generateForStmt(const AST::ForStmt *forStmt)
{
	// Generate initializer
	if (forStmt->initializer)
	{
		generateStatement(forStmt->initializer.get());
	}

	int loopStartIndex = static_cast<int>(bytecode.instructions.size());

	// Push loop context
	loopStartStack.push_back(loopStartIndex);
	breakJumpsStack.emplace_back();
	continueJumpsStack.emplace_back();

	// Generate condition (if present)
	int jumpToEndIndex = -1;
	if (forStmt->condition)
	{
		generateExpression(forStmt->condition.get());
		jumpToEndIndex = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP_IF_FALSE, 0);
	}

	// Generate body
	generateStatement(forStmt->body.get());

	// Continue jumps to increment (or loop start if no increment)
	int incrementIndex = static_cast<int>(bytecode.instructions.size());
	for (int continueJump : continueJumpsStack.back())
	{
		bytecode.instructions[continueJump].operand1 = incrementIndex;
	}

	// Generate increment
	if (forStmt->increment)
	{
		if (const auto *postfix = dynamic_cast<const AST::PostfixExpr *>(forStmt->increment.get()))
		{
			// resultNeeded=false: skips the old-value LOAD_VAR, and STORE_VAR
			// already pops the result — stack is clean, no POP needed.
			generatePostfixExpr(postfix, false);
		}
		else
		{
			generateExpression(forStmt->increment.get());
			bytecode.emit(OpCode::POP); // discard result
		}
	}

	// Jump back to condition check
	bytecode.emit(OpCode::JUMP_BACK, loopStartIndex);

	// Patch jump to end and break jumps
	int endIndex = static_cast<int>(bytecode.instructions.size());
	if (jumpToEndIndex != -1)
	{
		bytecode.instructions[jumpToEndIndex].operand1 = endIndex;
	}
	for (int breakJump : breakJumpsStack.back())
	{
		bytecode.instructions[breakJump].operand1 = endIndex;
	}

	// Pop loop context
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
	// Emit jump with placeholder, will be patched later
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
	// Emit jump with placeholder, will be patched later
	int jumpIndex = static_cast<int>(bytecode.instructions.size());
	bytecode.emit(OpCode::JUMP, 0);
	continueJumpsStack.back().push_back(jumpIndex);
}

void CodeGenerator::generateReturnStmt(const AST::ReturnStmt *returnStmt)
{
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
				if (retType != ValueType::Array)
				{
					ValueType expectedType = mapTypeNameToValueType(currentFunctionReturnType);
					if (retType != expectedType)
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
	// Jump over function body
	int jumpOverIndex = static_cast<int>(bytecode.instructions.size());
	bytecode.emit(OpCode::JUMP, 0);

	// Record entry point
	int entryPoint = static_cast<int>(bytecode.instructions.size());
	bytecode.functionEntries[funcDecl->name] = entryPoint;

	// Record parameter count
	bytecode.functionParamCounts[funcDecl->name] = static_cast<int>(funcDecl->params.size());

	std::vector<std::string> paramTypeNames;
	paramTypeNames.reserve(funcDecl->params.size());
	for (const auto &p : funcDecl->params)
		paramTypeNames.push_back(p.type ? p.type->name : "any");
	bytecode.functionParamTypeNames[funcDecl->name] = std::move(paramTypeNames);

	std::string prevReturnType = currentFunctionReturnType;
	currentFunctionReturnType = (funcDecl->returnType ? funcDecl->returnType->name : "any");
	bytecode.functionReturnTypeNames[funcDecl->name] = currentFunctionReturnType;
	bytecode.emit(OpCode::POP);

	// Pop parameters in reverse order and store in variables
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
				inferredTypes[it->name]  = ValueType::Array;
			}
			else
			{
				inferredTypes[it->name] = mapTypeNameToValueType(it->type->name);
			}
		}
	}

	generateBlockStmt(funcDecl->body.get());

	if (bytecode.instructions.empty() || bytecode.instructions.back().op != OpCode::RETURN)
	{
		if (!currentFunctionReturnType.empty() &&
		    currentFunctionReturnType != "any"  &&
		    currentFunctionReturnType != "void")
		{
			throw std::runtime_error("ERROR: function '" + funcDecl->name + "' is declared to return '"
			          + currentFunctionReturnType + "' but has no return statement.\n");
		}
		bytecode.emit(OpCode::NULL_VAL);
		bytecode.emit(OpCode::RETURN);
	}

	currentFunctionReturnType = prevReturnType;

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
        // Variable assignment: a = value
        
        // Validation: If assigning a raw array literal to a statically typed array
        auto arrayIt = arrayBaseTypes.find(identExpr->name);
        if (arrayIt != arrayBaseTypes.end())
        {
            std::string expectedBaseType = arrayIt->second;
            if (expectedBaseType != "any")
            {
                ValueType expectedElemType = mapTypeNameToValueType(expectedBaseType);
                if (const auto *arrayLit = dynamic_cast<const AST::ArrayLiteralExpr *>(assignExpr->value.get()))
                {
                    for (size_t i = 0; i < arrayLit->elements.size(); ++i)
                    {
                        bool known = false;
                        ValueType elemType = inferExpressionType(arrayLit->elements[i].get(), known);
                        if (known && elemType != expectedElemType)
                        {
                            throw std::runtime_error("Type mismatch in array assignment: element at index " +
                                                     std::to_string(i) + " does not match expected base type '" +
                                                     expectedBaseType + "'.");
                        }
                    }
                }
            }
        }

        generateExpression(assignExpr->value.get());

        // propagate type
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

        int fieldNameIndex = bytecode.addStringConstant(fieldExpr->fieldName);
        bytecode.emit(OpCode::SET_FIELD, fieldNameIndex);
        bytecode.emit(OpCode::GET_FIELD, fieldNameIndex);
    }
    else if (const auto *arrayAccess = dynamic_cast<const AST::ArrayAccessExpr *>(assignExpr->target.get()))
    {
        // Element-wise assignment: a[i] = value
        
        // Validation: If indexing into a statically typed array
        if (const auto *identExpr = dynamic_cast<const AST::IdentifierExpr *>(arrayAccess->array.get()))
        {
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

        generateExpression(arrayAccess->array.get());   // array
        generateExpression(arrayAccess->index.get());   // index
        generateExpression(assignExpr->value.get());    // value
        
        int countIdx = bytecode.addConstant(Value(static_cast<i64>(3)));
        bytecode.emit(OpCode::PUSH_CONST, countIdx);    // count for __set_elem
        
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
	// Prefer static struct instantiation when we have metadata
	auto it = bytecode.structEntries.find(expr->structName);
	if (it != bytecode.structEntries.end())
	{
		int structIndex = it->second;

		// Create instance with defaults from Bytecode::structs / constants
		bytecode.emit(OpCode::NEW_STRUCT_INSTANCE_STATIC, structIndex);

		// Apply any explicit field initializers as overrides using dynamic SET_FIELD
		for (const auto &[fieldName, fieldValue] : expr->fieldValues)
		{
			generateExpression(fieldValue.get());
			int fieldNameIndex = bytecode.addStringConstant(fieldName);
			bytecode.emit(OpCode::SET_FIELD, fieldNameIndex);
		}
	}
	else
	{
		// Fallback
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

	// Propagate inferred type to temp var
	bool known = false;
	ValueType inferred = inferExpressionType(switchStmt->expr.get(), known);
	if (known)
	{
		inferredTypes[tempName] = inferred;
	}

	std::vector<int> endJumps; // one per case, all patched to end

	for (const auto &caseClause : switchStmt->cases)
	{
		// Reload switch value for every comparison
		bytecode.emit(OpCode::LOAD_VAR, tempVarIndex);
		generateExpression(caseClause.value.get());
		bytecode.emit(OpCode::FLEQUAL);

		int skipJump = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP_IF_FALSE, 0); // skip this case if no match

		for (const auto &stmt : caseClause.statements)
		{
			generateStatement(stmt.get());
		}

		int endJump = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP, 0); // after executing, jump to end
		endJumps.push_back(endJump);

		// Patch skip jump to point at the next case
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
    generateExpression(arrayAccess->array.get());   // array
    generateExpression(arrayAccess->index.get());   // index
    int countIdx = bytecode.addConstant(Value(static_cast<i64>(2)));
    bytecode.emit(OpCode::PUSH_CONST, countIdx);    // count for __get_elem
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
    // NOTE: "array" is no longer a valid standalone type name.
    // Arrays are declared via the T[] suffix (e.g. int[], string[], any[]).
    if (typeName == "struct") [[unlikely]]        return ValueType::Struct;
    if (bytecode.structEntries.find(typeName) != bytecode.structEntries.end())
    { [[likely]] return ValueType::Struct; }
    if (typeName == "void")                       return ValueType::Null;

    return ValueType::Float;
}

} // namespace Phasor