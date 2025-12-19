#include "VM.hpp"
#include <iostream>
#include <stdexcept>
#include "core/core.h"

void VM::run(const Bytecode &bc)
{
	this->bytecode = &bc;
	pc = 0;
	stack.clear();
	callStack.clear();
	operandStack.clear();

	registers.fill(Value());
	variables.resize(bytecode->nextVarIndex);

	while (pc < bytecode->instructions.size())
	{
		const Instruction &instr = bytecode->instructions[pc++];
		operation(instr.op, instr.operand1, instr.operand2, instr.operand3, instr.operand4, instr.operand5);
	}
}

Value VM::operation(const OpCode &op, const int &operand1, const int &operand2, const int &operand3,
                    const int &operand4, const int &operand5)
{
	uint8_t rA = static_cast<uint8_t>(operand1);
	uint8_t rB = static_cast<uint8_t>(operand2);
	uint8_t rC = static_cast<uint8_t>(operand3);
	switch (op)
	{
	case OpCode::PUSH_CONST:
		if (operand1 < 0 || operand1 >= static_cast<int>(bytecode->constants.size()))
			throw std::runtime_error("Invalid constant index");
		push(bytecode->constants[operand1]);
		break;

	case OpCode::POP:
		pop();
		break;

	case OpCode::ADD: {
		Value b = pop();
		Value a = pop();
		push(asm_add(a.asInt(), b.asInt()));
		break;
	}

	case OpCode::SUBTRACT: {
		Value b = pop();
		Value a = pop();
		push(asm_sub(a.asInt(), b.asInt()));
		break;
	}

	case OpCode::MULTIPLY: {
		Value b = pop();
		Value a = pop();
		push(asm_mul(a.asInt(), b.asInt()));
		break;
	}

	case OpCode::DIVIDE: {
		Value b = pop();
		Value a = pop();
		push(asm_div(a.asInt(), b.asInt()));
		break;
	}

	case OpCode::MODULO: {
		Value b = pop();
		Value a = pop();
		push(asm_mod(a.asInt(), b.asInt()));
		break;
	}

	case OpCode::SQRT: {
		Value a = pop();
		push(asm_sqrt(a.asFloat()));
		break;
	}

	case OpCode::POW: {
		Value b = pop();
		Value a = pop();
		push(asm_pow(a.asFloat(), b.asFloat()));
		break;
	}

	case OpCode::LOG: {
		Value a = pop();
		push(asm_log(a.asFloat()));
		break;
	}

	case OpCode::EXP: {
		Value a = pop();
		push(asm_exp(a.asFloat()));
		break;
	}

	case OpCode::SIN: {
		Value a = pop();
		push(asm_sin(a.asFloat()));
		break;
	}

	case OpCode::COS: {
		Value a = pop();
		push(asm_cos(a.asFloat()));
		break;
	}

	case OpCode::TAN: {
		Value a = pop();
		push(asm_tan(a.asFloat()));
		break;
	}

	case OpCode::NEGATE:
		push(asm_neg(pop().asInt()));
		break;

	case OpCode::NOT:
		push(Value(asm_not(pop().isTruthy() ? 1 : 0)));
		break;

	case OpCode::AND: {
		Value b = pop();
		Value a = pop();
		push(Value(asm_and(a.isTruthy() ? 1 : 0, b.isTruthy() ? 1 : 0)));
		break;
	}

	case OpCode::OR: {
		Value b = pop();
		Value a = pop();
		push(Value(asm_or(a.isTruthy() ? 1 : 0, b.isTruthy() ? 1 : 0)));
		break;
	}

	case OpCode::EQUAL: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_equal(a.asInt(), b.asInt())) : Value(a == b));
		break;
	}

	case OpCode::NOT_EQUAL: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_not_equal(a.asInt(), b.asInt())) : Value(a != b));
		break;
	}

	case OpCode::LESS_THAN: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_less_than(a.asInt(), b.asInt())) : Value(a < b));
		break;
	}

	case OpCode::GREATER_THAN: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_greater_than(a.asInt(), b.asInt())) : Value(a > b));
		break;
	}

	case OpCode::LESS_EQUAL: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_less_equal(a.asInt(), b.asInt())) : Value(a <= b));
		break;
	}

	case OpCode::GREATER_EQUAL: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_greater_equal(a.asInt(), b.asInt())) : Value(a >= b));
		break;
	}

	case OpCode::JUMP:
		pc = operand1;
		break;

	case OpCode::JUMP_IF_FALSE:
		if (!pop().isTruthy())
			pc = operand1;
		break;

	case OpCode::JUMP_IF_TRUE:
		if (pop().isTruthy())
			pc = operand1;
		break;

	case OpCode::JUMP_BACK:
		pc = operand1;
		break;

	case OpCode::STORE_VAR:
		if (operand1 < 0 || operand1 >= static_cast<int>(variables.size()))
			throw std::runtime_error("Invalid variable index");
		variables[operand1] = pop();
		break;

	case OpCode::LOAD_VAR:
		if (operand1 < 0 || operand1 >= static_cast<int>(variables.size()))
			throw std::runtime_error("Invalid variable index");
		push(variables[operand1]);
		break;

	case OpCode::PRINT: {
		Value       v = pop();
		std::string s = v.toString();
		asm_print_stdout(s.c_str(), s.length());
		break;
	}

	case OpCode::PRINTERROR: {
		Value       v = pop();
		std::string s = v.toString();
		asm_print_stderr(s.c_str(), s.length());
		break;
	}

	case OpCode::READLINE: {
		std::string s;
		std::getline(std::cin, s);
		push(s);
		break;
	}

	case OpCode::IMPORT: {
		Value       pathVal = bytecode->constants[operand1];
		std::string path = pathVal.asString();
		if (importHandler)
			importHandler(path);
		else
			throw std::runtime_error("Import handler not set");
		break;
	}
	case OpCode::HALT:
		pc = bytecode->instructions.size(); // stop execution
		break;

	case OpCode::CALL_NATIVE: {
		Value       funcNameVal = bytecode->constants[operand1];
		std::string funcName = funcNameVal.asString();
		auto        it = nativeFunctions.find(funcName);
		if (it == nativeFunctions.end())
			throw std::runtime_error("Unknown native function: " + funcName);

		int                argCount = static_cast<int>(pop().asInt());
		std::vector<Value> args(argCount);
		for (int i = argCount - 1; i >= 0; --i)
			args[i] = pop();

		push(it->second(args, this));
		break;
	}
	case OpCode::TRUE:
		push(Value(true));
		break;

	case OpCode::FALSE:
		push(Value(false));
		break;

	case OpCode::NULL_VAL:
		push(Value());
		break;

	case OpCode::CALL: {
		Value       funcNameVal = bytecode->constants[operand1];
		std::string funcName = funcNameVal.asString();
		auto        it = bytecode->functionEntries.find(funcName);
		if (it == bytecode->functionEntries.end())
			throw std::runtime_error("Unknown function: " + funcName);

		callStack.push_back(static_cast<int>(pc));
		pc = it->second;
		break;
	}
	case OpCode::RETURN: {
		if (callStack.empty())
		{
			pc = bytecode->instructions.size();
			break;
		}
		Value returnValue = pop();
		pc = callStack.back();
		callStack.pop_back();
		push(returnValue);
		break;
	}

	case OpCode::SYSTEM: {
		asm_system(registers[rA].c_str());
		break;
	}

	// Register-based operations (v2.0)
	case OpCode::MOV: {
		registers[rA] = registers[rB];
		break;
	}

	case OpCode::LOAD_CONST_R: {
		// LOAD_CONST_R rA, constIndex
		int constIndex = operand2;
		if (constIndex < 0 || constIndex >= static_cast<int>(bytecode->constants.size()))
			throw std::runtime_error("Invalid constant index");
		registers[rA] = bytecode->constants[constIndex];
		break;
	}

	case OpCode::LOAD_VAR_R: {
		// LOAD_VAR_R rA, varIndex
		int varIndex = operand2;
		if (varIndex < 0 || varIndex >= static_cast<int>(variables.size()))
			throw std::runtime_error("Invalid variable index");
		registers[rA] = variables[varIndex];
		break;
	}

	case OpCode::STORE_VAR_R: {
		// STORE_VAR_R rA, varIndex - store register rA to variable varIndex
		int varIndex = operand2;
		if (varIndex < 0 || varIndex >= static_cast<int>(variables.size()))
			throw std::runtime_error("Invalid variable index");
		variables[varIndex] = registers[rA];
		break;
	}

	// Register arithmetic (3-address code)
	// Format: operand1=rA (dest), operand2=rB, operand3=rC
	case OpCode::ADD_R: {
		registers[rA] = Value(asm_add(registers[rB].asInt(), registers[rC].asInt()));
		break;
	}

	case OpCode::SUB_R: {
		registers[rA] = Value(asm_sub(registers[rB].asInt(), registers[rC].asInt()));
		break;
	}

	case OpCode::MUL_R: {
		registers[rA] = Value(asm_mul(registers[rB].asInt(), registers[rC].asInt()));
		break;
	}

	case OpCode::DIV_R: {
		registers[rA] = Value(asm_div(registers[rB].asInt(), registers[rC].asInt()));
		break;
	}

	case OpCode::MOD_R: {
		registers[rA] = Value(asm_mod(registers[rB].asInt(), registers[rC].asInt()));
		break;
	}

	case OpCode::SQRT_R: {
		registers[rA] = Value(asm_sqrt(registers[rB].asFloat()));
		break;
	}

	case OpCode::POW_R: {
		registers[rA] = Value(asm_pow(registers[rB].asFloat(), registers[rC].asFloat()));
		break;
	}

	case OpCode::LOG_R: {
		registers[rA] = Value(asm_log(registers[rB].asFloat()));
		break;
	}

	case OpCode::EXP_R: {
		registers[rA] = Value(asm_exp(registers[rB].asFloat()));
		break;
	}

	case OpCode::SIN_R: {
		registers[rA] = Value(asm_sin(registers[rB].asFloat()));
		break;
	}

	case OpCode::COS_R: {
		registers[rA] = Value(asm_cos(registers[rB].asFloat()));
		break;
	}

	case OpCode::TAN_R: {
		registers[rA] = Value(asm_tan(registers[rB].asFloat()));
		break;
	}

	// Register comparisons
	// Format: operand1=rA (dest), operand2=rB, operand3=rC
	case OpCode::EQ_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_equal(b.asInt(), c.asInt())) : Value(b == c);
		break;
	}

	case OpCode::NE_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_not_equal(b.asInt(), c.asInt())) : Value(b != c);
		break;
	}

	case OpCode::LT_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_less_than(b.asInt(), c.asInt())) : Value(b < c);
		break;
	}

	case OpCode::GT_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_greater_than(b.asInt(), c.asInt())) : Value(b > c);
		break;
	}

	case OpCode::LE_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_less_equal(b.asInt(), c.asInt())) : Value(b <= c);
		break;
	}

	case OpCode::GE_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_greater_equal(b.asInt(), c.asInt())) : Value(b >= c);
		break;
	}

	case OpCode::AND_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = Value(asm_and(b.isTruthy() ? 1 : 0, c.isTruthy() ? 1 : 0));
		break;
	}

	case OpCode::OR_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = Value(asm_or(b.isTruthy() ? 1 : 0, c.isTruthy() ? 1 : 0));
		break;
	}

	// Register-stack interaction
	case OpCode::PUSH_R: {
		push(registers[rA]);
		break;
	}

	case OpCode::PUSH2_R: {
		push(registers[rA]);
		push(registers[rB]);
		break;
	}

	case OpCode::POP_R: {
		registers[rA] = pop();
		break;
	}

	case OpCode::POP2_R: {
		registers[rA] = pop();
		registers[rB] = pop();
		break;
	}

	// Register unary operations
	case OpCode::NEG_R: {
		// NEG_R rA, rB - rA = -rB
		registers[rA] = Value(asm_neg(registers[rB].asInt()));
		break;
	}

	case OpCode::NOT_R: {
		// NOT_R rA, rB - rA = !rB
		registers[rA] = Value(asm_not(registers[rB].isTruthy() ? 1 : 0));
		break;
	}

	// Register I/O
	case OpCode::PRINT_R: {
		std::string s = registers[rA].toString();
		asm_print_stdout(s.c_str(), s.length());
		break;
	}

	case OpCode::PRINTERROR_R: {
		asm_print_stderr(registers[rA].c_str(), registers[rA].toString().length());
		break;
	}

	case OpCode::READLINE_R: {
		std::string s;
		std::getline(std::cin, s);
		registers[rA] = s;
		break;
	}

	case OpCode::SYSTEM_R: {
		registers[rA] = asm_system(registers[rA].c_str());
		break;
	}

	case OpCode::LEN: {
		Value v = pop();
		if (v.isString())
			push(Value(static_cast<int64_t>(v.asString().length())));
		else
			throw std::runtime_error("len() expects a string");
		break;
	}

	case OpCode::CHAR_AT: {
		Value idxVal = pop();
		Value strVal = pop();
		if (strVal.isString() && idxVal.isInt())
		{
			const std::string &s = strVal.asString();
			int64_t            idx = idxVal.asInt();
			if (idx < 0 || idx >= static_cast<int64_t>(s.length()))
				push(Value(""));
			else
				push(Value(std::string(1, s[idx])));
		}
		else
		{
			throw std::runtime_error("char_at() expects string and integer");
		}
		break;
	}

	case OpCode::SUBSTR: {
		Value lenVal = pop();
		Value startVal = pop();
		Value strVal = pop();

		if (strVal.isString() && startVal.isInt() && lenVal.isInt())
		{
			const std::string &s = strVal.asString();
			int64_t            start = startVal.asInt();
			int64_t            len = lenVal.asInt();

			if (start < 0 || start >= static_cast<int64_t>(s.length()))
			{
				push(Value(""));
			}
			else
			{
				push(Value(s.substr(start, len)));
			}
		}
		else
		{
			throw std::runtime_error("substr() expects string, int, int");
		}
		break;
	}

	case OpCode::NEW_STRUCT_INSTANCE_STATIC: {
		// operand1: structIndex into bytecode->structs
		if (operand1 < 0 || operand1 >= static_cast<int>(bytecode->structs.size()))
			throw std::runtime_error("Invalid struct index for NEW_STRUCT_INSTANCE_STATIC");

		const StructInfo &info = bytecode->structs[operand1];
		// Create struct instance by name, then apply default values from constants
		Value instance = Value::createStruct(info.name);
		for (int i = 0; i < info.fieldCount; ++i)
		{
			int constIndex = info.firstConstIndex + i;
			if (constIndex < 0 || constIndex >= static_cast<int>(bytecode->constants.size()))
				throw std::runtime_error("Invalid default constant index for struct field");
			const Value &defVal = bytecode->constants[constIndex];
			const std::string &fieldName = info.fieldNames[i];
			instance.setField(fieldName, defVal);
		}
		push(instance);
		break;
	}

	case OpCode::GET_FIELD_STATIC: {
		// operand1: structIndex, operand2: fieldOffset
		if (operand1 < 0 || operand1 >= static_cast<int>(bytecode->structs.size()))
			throw std::runtime_error("Invalid struct index for GET_FIELD_STATIC");
		const StructInfo &info = bytecode->structs[operand1];
		int fieldOffset = operand2;
		if (fieldOffset < 0 || fieldOffset >= info.fieldCount)
			throw std::runtime_error("Invalid field offset for GET_FIELD_STATIC");
		const std::string &fieldName = info.fieldNames[fieldOffset];
		Value obj = pop();
		push(obj.getField(fieldName));
		break;
	}

	case OpCode::SET_FIELD_STATIC: {
		// operand1: structIndex, operand2: fieldOffset
		if (operand1 < 0 || operand1 >= static_cast<int>(bytecode->structs.size()))
			throw std::runtime_error("Invalid struct index for SET_FIELD_STATIC");
		const StructInfo &info = bytecode->structs[operand1];
		int fieldOffset = operand2;
		if (fieldOffset < 0 || fieldOffset >= info.fieldCount)
			throw std::runtime_error("Invalid field offset for SET_FIELD_STATIC");
		const std::string &fieldName = info.fieldNames[fieldOffset];
		Value value = pop();
		Value obj = pop();
		obj.setField(fieldName, value);
		push(obj);
		break;
	}

	case OpCode::NEW_STRUCT: {
		if (operand1 < 0 || operand1 >= static_cast<int>(bytecode->constants.size()))
			throw std::runtime_error("Invalid constant index for NEW_STRUCT");
		Value nameVal = bytecode->constants[operand1];
		std::string structName = nameVal.asString();
		push(Value::createStruct(structName));
		break;
	}

	case OpCode::SET_FIELD: {
		if (operand1 < 0 || operand1 >= static_cast<int>(bytecode->constants.size()))
			throw std::runtime_error("Invalid constant index for SET_FIELD");
		std::string fieldName = bytecode->constants[operand1].asString();
		Value value = pop();
		Value obj = pop();
		obj.setField(fieldName, value);
		push(obj);
		break;
	}

	case OpCode::GET_FIELD: {
		if (operand1 < 0 || operand1 >= static_cast<int>(bytecode->constants.size()))
			throw std::runtime_error("Invalid constant index for GET_FIELD");
		std::string fieldName = bytecode->constants[operand1].asString();
		Value obj = pop();
		push(obj.getField(fieldName));
		break;
	}

	default:
		throw std::runtime_error("Unknown opcode");
	}
	return Value(operand1);
}

void VM::registerNativeFunction(const std::string &name, NativeFunction fn)
{
	nativeFunctions[name] = fn;
}

void VM::setImportHandler(ImportHandler handler)
{
	importHandler = handler;
}

void VM::push(const Value &value)
{
	stack.push_back(value);
}

Value VM::pop()
{
	if (stack.empty())
	{
		throw std::runtime_error("Stack underflow");
	}
	Value value = stack.back();
	stack.pop_back();
	return value;
}

Value VM::peek()
{
	if (stack.empty())
	{
		throw std::runtime_error("Stack is empty");
	}
	return stack.back();
}

void VM::reset(const bool &resetStack, const bool &resetFunctions, const bool &resetVariables)
{
	if (resetStack)
	{
		stack.clear();
		callStack.clear();
		operandStack.clear();
	}
	if (resetFunctions)
	{
		nativeFunctions.clear();
	}
	if (resetVariables)
	{
		variables.clear();
	}
	pc = 0;
	bytecode = nullptr;
}

size_t VM::addVariable(const Value &value)
{
	variables.push_back(value);
	return variables.size() - 1;
}

void VM::freeVariable(const size_t index)
{
	if (index < variables.size())
	{
		variables[index] = Value(); // Reset to null
	}
}

void VM::setVariable(const size_t index, const Value &value)
{
	if (index >= variables.size())
	{
		throw std::runtime_error("Invalid variable index");
	}
	variables[index] = value;
}

Value VM::getVariable(const size_t index)
{
	if (index >= variables.size())
	{
		throw std::runtime_error("Invalid variable index");
	}
	return variables[index];
}

size_t VM::getVariableCount()
{
	return variables.size();
}

void VM::setRegister(const uint8_t index, const Value &value)
{
	registers[index] = value;
}

void VM::freeRegister(const uint8_t index)
{
	registers[index] = Value(); // Reset to null
}

Value VM::getRegister(const uint8_t index)
{
	return registers[index];
}

size_t VM::getRegisterCount()
{
	return registers.size();
}

std::string VM::getInformation()
{
	int    callStackTop = callStack.empty() ? -1 : callStack.back();
	size_t programCounter = pc;

	std::string info = "Stack Top: ";
	info += peek().toString();
	info += " | R0: ";
	info += registers[0].toString();
	info += " | R1: ";
	info += registers[1].toString();
	info += " | R2: ";
	info += registers[2].toString();
	info += " | R3: ";
	info += registers[3].toString();
	info += " | Current Program Counter: " + std::to_string(programCounter);
	info += " | PC Stack Top: " + std::to_string(callStackTop);
	return info;
}