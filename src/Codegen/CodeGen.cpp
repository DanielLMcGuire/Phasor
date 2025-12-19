#include "CodeGen.hpp"
#include <iostream>

Bytecode CodeGenerator::generate(const Program &program, const std::map<std::string, int> &existingVars, int nextVarIdx,
                                 bool replMode)
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

bool CodeGenerator::isLiteralExpression(const Expression *expr, Value &outValue)
{
	if (auto numExpr = dynamic_cast<const NumberExpr *>(expr))
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
	if (auto strExpr = dynamic_cast<const StringExpr *>(expr))
	{
		outValue = Value(strExpr->value);
		return true;
	}
	if (auto boolExpr = dynamic_cast<const BooleanExpr *>(expr))
	{
		outValue = Value(boolExpr->value);
		return true;
	}
	if (dynamic_cast<const NullExpr *>(expr))
	{
		outValue = Value();
		return true;
	}
	return false;
}

void CodeGenerator::generateStatement(const Statement *stmt)
{
	if (auto varDecl = dynamic_cast<const VarDecl *>(stmt))
	{
		generateVarDecl(varDecl);
	}
	else if (auto exprStmt = dynamic_cast<const ExpressionStmt *>(stmt))
	{
		generateExpressionStmt(exprStmt);
	}
	else if (auto printStmt = dynamic_cast<const PrintStmt *>(stmt))
	{
		generatePrintStmt(printStmt);
	}

	else if (auto importStmt = dynamic_cast<const ImportStmt *>(stmt))
	{
		generateImportStmt(importStmt);
	}
	else if (auto exportStmt = dynamic_cast<const ExportStmt *>(stmt))
	{
		generateExportStmt(exportStmt);
	}
	else if (auto blockStmt = dynamic_cast<const BlockStmt *>(stmt))
	{
		generateBlockStmt(blockStmt);
	}
	else if (auto ifStmt = dynamic_cast<const IfStmt *>(stmt))
	{
		generateIfStmt(ifStmt);
	}
	else if (auto whileStmt = dynamic_cast<const WhileStmt *>(stmt))
	{
		generateWhileStmt(whileStmt);
	}
	else if (auto forStmt = dynamic_cast<const ForStmt *>(stmt))
	{
		generateForStmt(forStmt);
	}
	else if (auto returnStmt = dynamic_cast<const ReturnStmt *>(stmt))
	{
		generateReturnStmt(returnStmt);
	}
	else if (auto unsafeStmt = dynamic_cast<const UnsafeBlockStmt *>(stmt))
	{
		generateUnsafeBlockStmt(unsafeStmt);
	}
	else if (auto funcDecl = dynamic_cast<const FunctionDecl *>(stmt))
	{
		generateFunctionDecl(funcDecl);
	}
	else if (auto structDecl = dynamic_cast<const StructDecl*>(stmt)) {
    	generateStructDecl(structDecl);
	}
	else if (dynamic_cast<const BreakStmt*>(stmt)) {
		generateBreakStmt();
	}
	else if (dynamic_cast<const ContinueStmt*>(stmt)) {
		generateContinueStmt();
	}
	else if (auto switchStmt = dynamic_cast<const SwitchStmt*>(stmt)) {
		generateSwitchStmt(switchStmt);
	}
	else
	{
		throw std::runtime_error("Unknown statement type in code generation");
	}
}

void CodeGenerator::generateExpression(const Expression *expr)
{
	if (auto numExpr = dynamic_cast<const NumberExpr *>(expr))
	{
		generateNumberExpr(numExpr);
	}
	else if (auto strExpr = dynamic_cast<const StringExpr *>(expr))
	{
		generateStringExpr(strExpr);
	}
	else if (auto identExpr = dynamic_cast<const IdentifierExpr *>(expr))
	{
		generateIdentifierExpr(identExpr);
	}
	else if (auto unaryExpr = dynamic_cast<const UnaryExpr *>(expr))
	{
		generateUnaryExpr(unaryExpr);
	}
	else if (auto callExpr = dynamic_cast<const CallExpr *>(expr))
	{
		generateCallExpr(callExpr);
	}
	else if (auto binExpr = dynamic_cast<const BinaryExpr *>(expr))
	{
		generateBinaryExpr(binExpr);
	}
	else if (auto boolExpr = dynamic_cast<const BooleanExpr *>(expr))
	{
		generateBooleanExpr(boolExpr);
	}
	else if (auto nullExpr = dynamic_cast<const NullExpr *>(expr))
	{
		generateNullExpr(nullExpr);
	}
	else if (auto assignExpr = dynamic_cast<const AssignmentExpr *>(expr))
	{
		generateAssignmentExpr(assignExpr);
	}
	else if (auto structExpr = dynamic_cast<const StructInstanceExpr*>(expr)) {
    	generateStructInstanceExpr(structExpr);
	}
	else if (auto fieldAccessExpr = dynamic_cast<const FieldAccessExpr*>(expr)) {
    	generateFieldAccessExpr(fieldAccessExpr);
	}
	else if (auto postfixExpr = dynamic_cast<const PostfixExpr*>(expr)) {
		generatePostfixExpr(postfixExpr);
	}
	else
	{
		throw std::runtime_error("Unknown expression type in code generation");
	}
}

void CodeGenerator::generateVarDecl(const VarDecl *varDecl)
{
	if (varDecl->initializer)
	{
		generateExpression(varDecl->initializer.get());
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

void CodeGenerator::generateExpressionStmt(const ExpressionStmt *exprStmt)
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

void CodeGenerator::generatePrintStmt(const PrintStmt *printStmt)
{
	generateExpression(printStmt->expression.get());
	bytecode.emit(OpCode::PRINT);
}

void CodeGenerator::generateImportStmt(const ImportStmt *importStmt)
{
	int constIndex = bytecode.addConstant(Value(importStmt->modulePath));
	bytecode.emit(OpCode::IMPORT, constIndex);
}

void CodeGenerator::generateExportStmt(const ExportStmt *exportStmt)
{
	// For now, export just executes the declaration in the current scope
	generateStatement(exportStmt->declaration.get());
}

void CodeGenerator::generateNumberExpr(const NumberExpr *numExpr)
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

void CodeGenerator::generateStringExpr(const StringExpr *strExpr)
{
	int constIndex = bytecode.addConstant(Value(strExpr->value));
	bytecode.emit(OpCode::PUSH_CONST, constIndex);
}

void CodeGenerator::generateIdentifierExpr(const IdentifierExpr *identExpr)
{
	int varIndex = bytecode.getOrCreateVar(identExpr->name);
	bytecode.emit(OpCode::LOAD_VAR, varIndex);
}

void CodeGenerator::generateUnaryExpr(const UnaryExpr *unaryExpr)
{
	// Generate operand
	generateExpression(unaryExpr->operand.get());

	// Emit operation
	switch (unaryExpr->op)
	{
	case UnaryOp::Negate:
		bytecode.emit(OpCode::NEGATE);
		break;
	case UnaryOp::Not:
		bytecode.emit(OpCode::NOT);
		break;
	}
}

void CodeGenerator::generateCallExpr(const CallExpr *callExpr)
{
	// Optimizations
	// Optimizations
	if (callExpr->callee == "len" && callExpr->arguments.size() == 1)
	{
		if (auto strExpr = dynamic_cast<const StringExpr *>(callExpr->arguments[0].get()))
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
		if (auto numExpr = dynamic_cast<const NumberExpr *>(callExpr->arguments[2].get()))
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
		auto s = dynamic_cast<const StringExpr *>(callExpr->arguments[0].get());
		auto p = dynamic_cast<const StringExpr *>(callExpr->arguments[1].get());
		if (s && p)
		{
			bool result =
			    s->value.length() >= p->value.length() && s->value.compare(0, p->value.length(), p->value) == 0;
			bytecode.emit(result ? OpCode::TRUE : OpCode::FALSE);
			return;
		}
	}

	if (callExpr->callee == "ends_with" && callExpr->arguments.size() == 2)
	{
		auto s = dynamic_cast<const StringExpr *>(callExpr->arguments[0].get());
		auto suffix = dynamic_cast<const StringExpr *>(callExpr->arguments[1].get());
		if (s && suffix)
		{
			bool result = s->value.length() >= suffix->value.length() &&
			              s->value.compare(s->value.length() - suffix->value.length(), suffix->value.length(),
			                               suffix->value) == 0;
			bytecode.emit(result ? OpCode::TRUE : OpCode::FALSE);
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
		bytecode.emit(OpCode::CALL, nameIndex);
	}
	else
	{
		int nameIndex = bytecode.addConstant(Value(callExpr->callee));
		bytecode.emit(OpCode::CALL_NATIVE, nameIndex);
	}
}

void CodeGenerator::generateBinaryExpr(const BinaryExpr *binExpr)
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
			case BinaryOp::Add:
				result = leftVal + rightVal;
				break;
			case BinaryOp::Subtract:
				result = leftVal - rightVal;
				break;
			case BinaryOp::Multiply:
				result = leftVal * rightVal;
				break;
			case BinaryOp::Divide:
				result = leftVal / rightVal;
				break;
			case BinaryOp::Modulo:
				result = leftVal % rightVal;
				break;
			case BinaryOp::And:
				result = leftVal.logicalAnd(rightVal);
				break;
			case BinaryOp::Or:
				result = leftVal.logicalOr(rightVal);
				break;
			case BinaryOp::Equal:
				result = Value(leftVal == rightVal);
				break;
			case BinaryOp::NotEqual:
				result = Value(leftVal != rightVal);
				break;
			case BinaryOp::LessThan:
				result = Value(leftVal < rightVal);
				break;
			case BinaryOp::GreaterThan:
				result = Value(leftVal > rightVal);
				break;
			case BinaryOp::LessEqual:
				result = Value(leftVal <= rightVal);
				break;
			case BinaryOp::GreaterEqual:
				result = Value(leftVal >= rightVal);
				break;
			}
			if (result.isBool())
			{
				if (result.asBool())
				{
					bytecode.emit(OpCode::TRUE);
				}
				else
				{
					bytecode.emit(OpCode::FALSE);
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

	if (binExpr->op == BinaryOp::And)
	{
		generateExpression(binExpr->left.get());
		int jumpToFalseIndex = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP_IF_FALSE, 0);
		generateExpression(binExpr->right.get());
		int jumpToEndIndex = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP, 0);
		bytecode.instructions[jumpToFalseIndex].operand1 = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::FALSE);
		bytecode.instructions[jumpToEndIndex].operand1 = static_cast<int>(bytecode.instructions.size());
		return;
	}
	if (binExpr->op == BinaryOp::Or)
	{
		generateExpression(binExpr->left.get());
		int jumpToTrueIndex = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP_IF_TRUE, 0);
		generateExpression(binExpr->right.get());
		int jumpToEndIndex = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP, 0);
		bytecode.instructions[jumpToTrueIndex].operand1 = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::TRUE);
		bytecode.instructions[jumpToEndIndex].operand1 = static_cast<int>(bytecode.instructions.size());
		return;
	}

	uint8_t rLeft = allocateRegister();
	uint8_t rRight = allocateRegister();
	uint8_t rResult = allocateRegister();

	Value leftLiteral;
	if (isLiteralExpression(binExpr->left.get(), leftLiteral))
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
	if (isLiteralExpression(binExpr->right.get(), rightLiteral))
	{
		int constIndex = bytecode.addConstant(rightLiteral);
		bytecode.emit(OpCode::LOAD_CONST_R, rRight, constIndex);
	}
	else
	{
		generateExpression(binExpr->right.get());
		bytecode.emit(OpCode::POP_R, rRight);
	}

	switch (binExpr->op)
	{
	case BinaryOp::Add:
		bytecode.emit(OpCode::ADD_R, rResult, rLeft, rRight);
		break;
	case BinaryOp::Subtract:
		bytecode.emit(OpCode::SUB_R, rResult, rLeft, rRight);
		break;
	case BinaryOp::Multiply:
		bytecode.emit(OpCode::MUL_R, rResult, rLeft, rRight);
		break;
	case BinaryOp::Divide:
		bytecode.emit(OpCode::DIV_R, rResult, rLeft, rRight);
		break;
	case BinaryOp::Modulo:
		bytecode.emit(OpCode::MOD_R, rResult, rLeft, rRight);
		break;
	case BinaryOp::And:
		bytecode.emit(OpCode::AND_R, rResult, rLeft, rRight);
		break;
	case BinaryOp::Or:
		bytecode.emit(OpCode::OR_R, rResult, rLeft, rRight);
		break;
	case BinaryOp::Equal:
		bytecode.emit(OpCode::EQ_R, rResult, rLeft, rRight);
		break;
	case BinaryOp::NotEqual:
		bytecode.emit(OpCode::NE_R, rResult, rLeft, rRight);
		break;
	case BinaryOp::LessThan:
		bytecode.emit(OpCode::LT_R, rResult, rLeft, rRight);
		break;
	case BinaryOp::GreaterThan:
		bytecode.emit(OpCode::GT_R, rResult, rLeft, rRight);
		break;
	case BinaryOp::LessEqual:
		bytecode.emit(OpCode::LE_R, rResult, rLeft, rRight);
		break;
	case BinaryOp::GreaterEqual:
		bytecode.emit(OpCode::GE_R, rResult, rLeft, rRight);
		break;
	}

	freeRegister(rLeft);
	freeRegister(rRight);
	bytecode.emit(OpCode::PUSH_R, rResult);
	freeRegister(rResult);
}

void CodeGenerator::generateBlockStmt(const BlockStmt *blockStmt)
{
	for (const auto &stmt : blockStmt->statements)
	{
		generateStatement(stmt.get());
	}
}

void CodeGenerator::generateIfStmt(const IfStmt *ifStmt)
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

void CodeGenerator::generateWhileStmt(const WhileStmt *whileStmt)
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

void CodeGenerator::generateForStmt(const ForStmt *forStmt)
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

void CodeGenerator::generateReturnStmt(const ReturnStmt *returnStmt)
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

void CodeGenerator::generateUnsafeBlockStmt(const UnsafeBlockStmt *unsafeStmt)
{
	generateBlockStmt(unsafeStmt->block.get());
}

void CodeGenerator::generateFunctionDecl(const FunctionDecl *funcDecl)
{
	// Jump over function body
	int jumpOverIndex = static_cast<int>(bytecode.instructions.size());
	bytecode.emit(OpCode::JUMP, 0);

	// Record entry point
	int entryPoint = static_cast<int>(bytecode.instructions.size());
	bytecode.functionEntries[funcDecl->name] = entryPoint;

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

void CodeGenerator::generateBooleanExpr(const BooleanExpr *boolExpr)
{
	if (boolExpr->value)
	{
		bytecode.emit(OpCode::TRUE);
	}
	else
	{
		bytecode.emit(OpCode::FALSE);
	}
}

void CodeGenerator::generateNullExpr(const NullExpr *nullExpr)
{
	bytecode.emit(OpCode::NULL_VAL);
}

void CodeGenerator::generateAssignmentExpr(const AssignmentExpr *assignExpr)
{
	// Support assignments to variables and struct fields.
	if (auto identExpr = dynamic_cast<const IdentifierExpr *>(assignExpr->target.get()))
	{
		// Variable assignment: a = value
		// 1. Generate value (pushes value to stack)
		generateExpression(assignExpr->value.get());

		// 2. Store in variable (pops value)
		int varIndex = bytecode.getOrCreateVar(identExpr->name);
		bytecode.emit(OpCode::STORE_VAR, varIndex);

		// 3. Load it back (assignment is an expression that returns the value)
		bytecode.emit(OpCode::LOAD_VAR, varIndex);
	}
	else if (auto fieldExpr = dynamic_cast<const FieldAccessExpr *>(assignExpr->target.get()))
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

void CodeGenerator::generateStructInstanceExpr(const StructInstanceExpr *expr)
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

void CodeGenerator::generateFieldAccessExpr(const FieldAccessExpr *expr)
{
	// Generate code for the object being accessed
	generateExpression(expr->object.get());

	// For now, we'll use dynamic field access since we don't have type information
	// In a future version, we could add type inference or require type annotations
	// to enable static field access
	int fieldNameIndex = bytecode.addConstant(Value(expr->fieldName));
	bytecode.emit(OpCode::GET_FIELD, fieldNameIndex);
}

void CodeGenerator::generatePostfixExpr(const PostfixExpr *expr)
{
	// Only support postfix on identifiers
	auto identExpr = dynamic_cast<const IdentifierExpr*>(expr->operand.get());
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
	if (expr->op == PostfixOp::Increment)
	{
		bytecode.emit(OpCode::ADD);
	}
	else
	{
		bytecode.emit(OpCode::SUBTRACT);
	}
	
	// Store new value
	bytecode.emit(OpCode::STORE_VAR, varIndex);
	bytecode.emit(OpCode::POP); // Pop the stored value
	// Old value is still on stack as return value
}

void CodeGenerator::generateStructDecl(const StructDecl* decl)
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

void CodeGenerator::generateSwitchStmt(const SwitchStmt *switchStmt)
{
	generateExpression(switchStmt->expr.get());
	
	std::vector<int> caseJumps;
	int defaultJump = -1;
	
	for (const auto& caseClause : switchStmt->cases) {
		generateExpression(caseClause.value.get());
		bytecode.emit(OpCode::EQUAL);
		int jumpIndex = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP_IF_FALSE, 0);
		caseJumps.push_back(jumpIndex);
		
		for (const auto& stmt : caseClause.statements) {
			generateStatement(stmt.get());
		}
		
		int endJump = static_cast<int>(bytecode.instructions.size());
		bytecode.emit(OpCode::JUMP, 0);
		bytecode.instructions[jumpIndex].operand1 = static_cast<int>(bytecode.instructions.size());
		caseJumps.back() = endJump;
	}
	
	if (!switchStmt->defaultStmts.empty()) {
		for (const auto& stmt : switchStmt->defaultStmts) {
			generateStatement(stmt.get());
		}
	}
	
	int endIndex = static_cast<int>(bytecode.instructions.size());
	for (int jumpIdx : caseJumps) {
		bytecode.instructions[jumpIdx].operand1 = endIndex;
	}
}
