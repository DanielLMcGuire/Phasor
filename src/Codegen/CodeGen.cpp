#include "CodeGen.hpp"
#include <iostream>

namespace Phasor
{

Bytecode CodeGenerator::generate(const AST::Program &program, const std::map<std::string, int> &existingVars,
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
	if (auto numExpr = dynamic_cast<const AST::NumberExpr *>(expr))
	{
		try
		{
			if (numExpr->value.find('.') != std::string::npos)
			{
				outValue = Value(std::stod(numExpr->value));
			}
			else
			{
				outValue = Value(static_cast<int64_t>(std::stoll(numExpr->value)));
			}
			return true;
		}
		catch (...)
		{
			return false;
		}
	}
	if (auto strExpr = dynamic_cast<const AST::StringExpr *>(expr))
	{
		outValue = Value(strExpr->value);
		return true;
	}
	if (auto boolExpr = dynamic_cast<const AST::BooleanExpr *>(expr))
	{
		outValue = Value(boolExpr->value);
		return true;
	}
	if (dynamic_cast<const AST::NullExpr *>(expr))
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
	if (auto ident = dynamic_cast<const AST::IdentifierExpr *>(expr))
	{
		auto it = inferredTypes.find(ident->name);
		if (it != inferredTypes.end())
		{
			known = true;
			return it->second;
		}
	}

	// Unknown
	known = false;
	return ValueType::Float; // default when unknown (not used unless known==true)
}

void CodeGenerator::generateStatement(const AST::Statement *stmt)
{
	if (auto varDecl = dynamic_cast<const AST::VarDecl *>(stmt))
	{
		generateVarDecl(varDecl);
	}
	else if (auto exprStmt = dynamic_cast<const AST::ExpressionStmt *>(stmt))
	{
		generateExpressionStmt(exprStmt);
	}
	else if (auto printStmt = dynamic_cast<const AST::PrintStmt *>(stmt))
	{
		generatePrintStmt(printStmt);
	}

	else if (auto importStmt = dynamic_cast<const AST::ImportStmt *>(stmt))
	{
		generateImportStmt(importStmt);
	}
	else if (auto exportStmt = dynamic_cast<const AST::ExportStmt *>(stmt))
	{
		generateExportStmt(exportStmt);
	}
	else if (auto blockStmt = dynamic_cast<const AST::BlockStmt *>(stmt))
	{
		generateBlockStmt(blockStmt);
	}
	else if (auto ifStmt = dynamic_cast<const AST::IfStmt *>(stmt))
	{
		generateIfStmt(ifStmt);
	}
	else if (auto whileStmt = dynamic_cast<const AST::WhileStmt *>(stmt))
	{
		generateWhileStmt(whileStmt);
	}
	else if (auto forStmt = dynamic_cast<const AST::ForStmt *>(stmt))
	{
		generateForStmt(forStmt);
	}
	else if (auto returnStmt = dynamic_cast<const AST::ReturnStmt *>(stmt))
	{
		generateReturnStmt(returnStmt);
	}
	else if (auto unsafeStmt = dynamic_cast<const AST::UnsafeBlockStmt *>(stmt))
	{
		generateUnsafeBlockStmt(unsafeStmt);
	}
	else if (auto funcDecl = dynamic_cast<const AST::FunctionDecl *>(stmt))
	{
		generateFunctionDecl(funcDecl);
	}
	else if (auto structDecl = dynamic_cast<const AST::StructDecl *>(stmt))
	{
		generateStructDecl(structDecl);
	}
	else if (dynamic_cast<const AST::BreakStmt *>(stmt))
	{
		generateBreakStmt();
	}
	else if (dynamic_cast<const AST::ContinueStmt *>(stmt))
	{
		generateContinueStmt();
	}
	else if (auto switchStmt = dynamic_cast<const AST::SwitchStmt *>(stmt))
	{
		generateSwitchStmt(switchStmt);
	}
	else
	{
		throw std::runtime_error("Unknown statement type in code generation");
	}
}

void CodeGenerator::generateExpression(const AST::Expression *expr)
{
	if (auto numExpr = dynamic_cast<const AST::NumberExpr *>(expr))
	{
		generateNumberExpr(numExpr);
	}
	else if (auto strExpr = dynamic_cast<const AST::StringExpr *>(expr))
	{
		generateStringExpr(strExpr);
	}
	else if (auto identExpr = dynamic_cast<const AST::IdentifierExpr *>(expr))
	{
		generateIdentifierExpr(identExpr);
	}
	else if (auto unaryExpr = dynamic_cast<const AST::UnaryExpr *>(expr))
	{
		generateUnaryExpr(unaryExpr);
	}
	else if (auto callExpr = dynamic_cast<const AST::CallExpr *>(expr))
	{
		generateCallExpr(callExpr);
	}
	else if (auto binExpr = dynamic_cast<const AST::BinaryExpr *>(expr))
	{
		generateBinaryExpr(binExpr);
	}
	else if (auto boolExpr = dynamic_cast<const AST::BooleanExpr *>(expr))
	{
		generateBooleanExpr(boolExpr);
	}
	else if (auto nullExpr = dynamic_cast<const AST::NullExpr *>(expr))
	{
		generateNullExpr(nullExpr);
	}
	else if (auto assignExpr = dynamic_cast<const AST::AssignmentExpr *>(expr))
	{
		generateAssignmentExpr(assignExpr);
	}
	else if (auto structExpr = dynamic_cast<const AST::StructInstanceExpr *>(expr))
	{
		generateStructInstanceExpr(structExpr);
	}
	else if (auto fieldAccessExpr = dynamic_cast<const AST::FieldAccessExpr *>(expr))
	{
		generateFieldAccessExpr(fieldAccessExpr);
	}
	else if (auto postfixExpr = dynamic_cast<const AST::PostfixExpr *>(expr))
	{
		generatePostfixExpr(postfixExpr);
	}
	else
	{
		throw std::runtime_error("Unknown expression type in code generation");
	}
}

void CodeGenerator::generateVarDecl(const AST::VarDecl *varDecl)
{
	if (varDecl->initializer)
	{
		// Generate initializer code
		generateExpression(varDecl->initializer.get());
		// Try to infer type from initializer
		Value initVal;
		if (isLiteralExpression(varDecl->initializer.get(), initVal))
		{
			inferredTypes[varDecl->name] = initVal.getType();
		}
		else if (auto ident = dynamic_cast<const AST::IdentifierExpr *>(varDecl->initializer.get()))
		{
			// propagate known type from another variable
			auto it = inferredTypes.find(ident->name);
			if (it != inferredTypes.end())
			{
				inferredTypes[varDecl->name] = it->second;
			}
		}

		int varIndex = bytecode.getOrCreateVar(varDecl->name);
		bytecode.emit(OpCode::STORE_VAR, varIndex);
	}
	else
	{
		// Store null value for uninitialized variable
		int constIndex = bytecode.addConstant(Value());
		bytecode.emit(OpCode::PUSH_CONST, constIndex);
		int varIndex = bytecode.getOrCreateVar(varDecl->name);
		bytecode.emit(OpCode::STORE_VAR, varIndex);
	}
}

void CodeGenerator::generateExpressionStmt(const AST::ExpressionStmt *exprStmt)
{
	generateExpression(exprStmt->expression.get());
	if (isRepl)
	{
		bytecode.emit(OpCode::PRINT);
	}
	else
	{
		bytecode.emit(OpCode::POP); // Expression statements discard result
	}
}

void CodeGenerator::generatePrintStmt(const AST::PrintStmt *printStmt)
{
	generateExpression(printStmt->expression.get());
	bytecode.emit(OpCode::PRINT);
}

void CodeGenerator::generateImportStmt(const AST::ImportStmt *importStmt)
{
	int constIndex = bytecode.addConstant(Value(importStmt->modulePath));
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
			double d = std::stod(numExpr->value);
			int    constIndex = bytecode.addConstant(Value(d));
			bytecode.emit(OpCode::PUSH_CONST, constIndex);
		}
		else
		{
			int64_t i = std::stoll(numExpr->value);
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
	// Optimizations
	if (callExpr->callee == "len" && callExpr->arguments.size() == 1)
	{
		if (auto strExpr = dynamic_cast<const AST::StringExpr *>(callExpr->arguments[0].get()))
		{
			// Constant fold len("literal")
			int64_t len = strExpr->value.length();
			int     constIndex = bytecode.addConstant(Value(len));
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
		if (auto numExpr = dynamic_cast<const AST::NumberExpr *>(callExpr->arguments[2].get()))
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
		auto s = dynamic_cast<const AST::StringExpr *>(callExpr->arguments[0].get());
		auto p = dynamic_cast<const AST::StringExpr *>(callExpr->arguments[1].get());
		if (s && p)
		{
			bool result =
			    s->value.length() >= p->value.length() && s->value.compare(0, p->value.length(), p->value) == 0;
			bytecode.emit(result ? OpCode::TRUE_P : OpCode::FALSE_P);
			return;
		}
	}

	if (callExpr->callee == "ends_with" && callExpr->arguments.size() == 2)
	{
		auto s = dynamic_cast<const AST::StringExpr *>(callExpr->arguments[0].get());
		auto suffix = dynamic_cast<const AST::StringExpr *>(callExpr->arguments[1].get());
		if (s && suffix)
		{
			bool result = s->value.length() >= suffix->value.length() &&
			              s->value.compare(s->value.length() - suffix->value.length(), suffix->value.length(),
			                               suffix->value) == 0;
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
	int constIndex = bytecode.addConstant(Value(static_cast<int64_t>(callExpr->arguments.size())));
	bytecode.emit(OpCode::PUSH_CONST, constIndex);

	// Emit CALL_NATIVE or CALL
	// Check if it's a user function
	if (bytecode.functionEntries.count(callExpr->callee))
	{
		int nameIndex = bytecode.addConstant(Value(callExpr->callee));
		// The best fix of all time: check if the function exists and has the correct number of parameters
		auto itParam = bytecode.functionParamCounts.find(callExpr->callee);
		if (itParam != bytecode.functionParamCounts.end())
		{
			int expected = itParam->second;
			int got = static_cast<int>(callExpr->arguments.size());
			if (expected != got)
			{
				std::cerr << "ERROR: calling function '" << callExpr->callee << "' with " << got
						  << " arguments but it expects " << expected << "\n";
				std::exit(1);
			}
		}
		bytecode.emit(OpCode::CALL, nameIndex);
	}
	else
	{
		int nameIndex = bytecode.addConstant(Value(callExpr->callee));
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

	uint8_t rLeft = allocateRegister();
	uint8_t rRight = allocateRegister();
	uint8_t rResult = allocateRegister();

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

	// Conservative integer decision:
	// treat operand as known-int if it's an integer literal or a variable previously inferred as Int.
	auto exprIsKnownInt = [&](const AST::Expression *e, bool isLiteral, const Value &lit) -> bool {
		if (isLiteral)
			return lit.isInt();
		// variable case
		if (auto ident = dynamic_cast<const AST::IdentifierExpr *>(e))
		{
			auto it = inferredTypes.find(ident->name);
			return it != inferredTypes.end() && it->second == ValueType::Int;
		}
		return false;
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

	// Jump to else if false
	int jumpToElseIndex = static_cast<int>(bytecode.instructions.size());
	bytecode.emit(OpCode::JUMP_IF_FALSE, 0);

	generateStatement(ifStmt->thenBranch.get());

	int jumpToEndIndex = static_cast<int>(bytecode.instructions.size());
	bytecode.emit(OpCode::JUMP, 0);

	// Patch jump to else
	bytecode.instructions[jumpToElseIndex].operand1 = static_cast<int>(bytecode.instructions.size());

	if (ifStmt->elseBranch)
	{
		generateStatement(ifStmt->elseBranch.get());
	}

	// Patch jump to end
	bytecode.instructions[jumpToEndIndex].operand1 = static_cast<int>(bytecode.instructions.size());
}

void CodeGenerator::generateWhileStmt(const AST::WhileStmt *whileStmt)
{
	int loopStartIndex = static_cast<int>(bytecode.instructions.size());

	// Push loop context
	loopStartStack.push_back(loopStartIndex);
	breakJumpsStack.push_back(std::vector<int>());
	continueJumpsStack.push_back(std::vector<int>());

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
	breakJumpsStack.push_back(std::vector<int>());
	continueJumpsStack.push_back(std::vector<int>());

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
		generateExpression(forStmt->increment.get());
		bytecode.emit(OpCode::POP); // Discard increment result
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
		generateExpression(returnStmt->value.get());
	}
	else
	{
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

	// Pop argument count (it's on the stack when function is called)
	bytecode.emit(OpCode::POP);

	// Pop parameters in reverse order and store in variables
	for (auto it = funcDecl->params.rbegin(); it != funcDecl->params.rend(); ++it)
	{
		int varIndex = bytecode.getOrCreateVar(it->name);
		bytecode.emit(OpCode::STORE_VAR, varIndex);
	}

	// Generate body
	generateBlockStmt(funcDecl->body.get());

	// Ensure return
	bytecode.emit(OpCode::NULL_VAL);
	bytecode.emit(OpCode::RETURN);

	// Patch jump over
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
	// Support assignments to variables and struct fields.
	if (auto identExpr = dynamic_cast<const AST::IdentifierExpr *>(assignExpr->target.get()))
	{
		// Variable assignment: a = value
		// 1. Generate value (pushes value to stack)
		generateExpression(assignExpr->value.get());

		// Try to infer and record type
		Value val;
		if (isLiteralExpression(assignExpr->value.get(), val))
		{
			inferredTypes[identExpr->name] = val.getType();
		}
		else if (auto identRhs = dynamic_cast<const AST::IdentifierExpr *>(assignExpr->value.get()))
		{
			auto it = inferredTypes.find(identRhs->name);
			if (it != inferredTypes.end())
				inferredTypes[identExpr->name] = it->second;
		}

		// 2. Store in variable (pops value)
		int varIndex = bytecode.getOrCreateVar(identExpr->name);
		bytecode.emit(OpCode::STORE_VAR, varIndex);

		// 3. Load it back (assignment is an expression that returns the value)
		bytecode.emit(OpCode::LOAD_VAR, varIndex);
	}
	else if (auto fieldExpr = dynamic_cast<const AST::FieldAccessExpr *>(assignExpr->target.get()))
	{
		// Generate object (pushes struct)
		generateExpression(fieldExpr->object.get());
		// Generate value (pushes value on top)
		generateExpression(assignExpr->value.get());

		// Use dynamic field access since we don't have type information
		int fieldNameIndex = bytecode.addConstant(Value(fieldExpr->fieldName));
		bytecode.emit(OpCode::SET_FIELD, fieldNameIndex);

		// Reload the assigned field value so the assignment expression evaluates to it
		generateExpression(fieldExpr->object.get());
		bytecode.emit(OpCode::GET_FIELD, fieldNameIndex);
	}
	else
	{
		throw std::runtime_error("Invalid assignment target. Only variables and struct fields are supported.");
	}
}

void CodeGenerator::generateStructInstanceExpr(const AST::StructInstanceExpr *expr)
{
	// Prefer static struct instantiation when we have metadata in the struct section.
	auto it = bytecode.structEntries.find(expr->structName);
	if (it != bytecode.structEntries.end())
	{
		int structIndex = it->second;
		// Create instance with defaults from Bytecode::structs / constants
		bytecode.emit(OpCode::NEW_STRUCT_INSTANCE_STATIC, structIndex);

		// Apply any explicit field initializers as overrides using dynamic SET_FIELD.
		for (const auto &[fieldName, fieldValue] : expr->fieldValues)
		{
			generateExpression(fieldValue.get());
			int fieldNameIndex = bytecode.addConstant(Value(fieldName));
			bytecode.emit(OpCode::SET_FIELD, fieldNameIndex);
		}
	}
	else
	{
		// Fallback: use the original dynamic struct creation path.
		int structNameIndex = bytecode.addConstant(Value(expr->structName));
		bytecode.emit(OpCode::NEW_STRUCT, structNameIndex);

		for (const auto &[fieldName, fieldValue] : expr->fieldValues)
		{
			generateExpression(fieldValue.get());
			int fieldNameIndex = bytecode.addConstant(Value(fieldName));
			bytecode.emit(OpCode::SET_FIELD, fieldNameIndex);
		}
	}
}

void CodeGenerator::generateFieldAccessExpr(const AST::FieldAccessExpr *expr)
{
	// Generate code for the object being accessed
	generateExpression(expr->object.get());

	// For now, we'll use dynamic field access since we don't have type information
	// In a future version, we could add type inference or require type annotations
	// to enable static field access
	int fieldNameIndex = bytecode.addConstant(Value(expr->fieldName));
	bytecode.emit(OpCode::GET_FIELD, fieldNameIndex);
}

void CodeGenerator::generatePostfixExpr(const AST::PostfixExpr *expr)
{
	// Only support postfix on identifiers
	auto identExpr = dynamic_cast<const AST::IdentifierExpr *>(expr->operand.get());
	if (!identExpr)
	{
		throw std::runtime_error("Postfix operators only supported on variables");
	}

	int varIndex = bytecode.getOrCreateVar(identExpr->name);

	// Load current value
	bytecode.emit(OpCode::LOAD_VAR, varIndex);

	// Duplicate for return value (postfix returns old value)
	bytecode.emit(OpCode::LOAD_VAR, varIndex);

	// Push 1
	int oneIndex = bytecode.addConstant(Value(static_cast<int64_t>(1)));
	bytecode.emit(OpCode::PUSH_CONST, oneIndex);

	// Add or subtract
	// Prefer integer ops if we have inferred this variable is int
	auto it = inferredTypes.find(identExpr->name);
	bool varIsInt = (it != inferredTypes.end() && it->second == ValueType::Int);
	if (expr->op == AST::PostfixOp::Increment)
	{
		bytecode.emit(varIsInt ? OpCode::IADD : OpCode::FLADD);
	}
	else
	{
		bytecode.emit(varIsInt ? OpCode::ISUBTRACT : OpCode::FLSUBTRACT);
	}

	// Store new value
	bytecode.emit(OpCode::STORE_VAR, varIndex);
	bytecode.emit(OpCode::POP); // Pop the stored value
	                            // Old value is still on stack as return value
}

void CodeGenerator::generateStructDecl(const AST::StructDecl *decl)
{
	// Store a human-readable struct definition in the constant pool (existing behavior)
	std::string structDef = "struct " + decl->name + " {";
	for (const auto &field : decl->fields)
	{
		structDef += " " + field.name + ":" + field.type->name + ",";
	}
	if (!decl->fields.empty())
	{
		structDef.pop_back(); // Remove trailing comma
	}
	structDef += " }";
	bytecode.addConstant(Value(structDef));

	// Register struct metadata in the struct section (no runtime use yet)
	// Allocate placeholder default constants (currently all null) for each field.
	int firstConstIndex = static_cast<int>(bytecode.constants.size());
	for (const auto &field : decl->fields)
	{
		(void)field; // field.type is not yet used for typed defaults
		bytecode.addConstant(Value());
	}

	StructInfo info;
	info.name = decl->name;
	info.firstConstIndex = firstConstIndex;
	info.fieldCount = static_cast<int>(decl->fields.size());
	for (const auto &field : decl->fields)
	{
		info.fieldNames.push_back(field.name);
	}

	// If a struct with this name already exists, do not overwrite it.
	if (bytecode.structEntries.find(decl->name) == bytecode.structEntries.end())
	{
		int index = static_cast<int>(bytecode.structs.size());
		bytecode.structs.push_back(std::move(info));
		bytecode.structEntries[decl->name] = index;
	}
}

void CodeGenerator::generateSwitchStmt(const AST::SwitchStmt *switchStmt)
{
	generateExpression(switchStmt->expr.get());

	std::vector<int> caseJumps;

	for (const auto &caseClause : switchStmt->cases)
	{
		generateExpression(caseClause.value.get());
		bytecode.emit(OpCode::FLEQUAL);
		int jumpIndex = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP_IF_FALSE, 0);
		caseJumps.push_back(jumpIndex);

		for (const auto &stmt : caseClause.statements)
		{
			generateStatement(stmt.get());
		}

		int endJump = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP, 0);
		bytecode.instructions[jumpIndex].operand1 = static_cast<int>(bytecode.instructions.size());
		caseJumps.back() = endJump;
	}

	if (!switchStmt->defaultStmts.empty())
	{
		for (const auto &stmt : switchStmt->defaultStmts)
		{
			generateStatement(stmt.get());
		}
	}

	int endIndex = static_cast<int>(bytecode.instructions.size());
	for (int jumpIdx : caseJumps)
	{
		bytecode.instructions[jumpIdx].operand1 = endIndex;
	}
}
} // namespace Phasor
