#include "VM.hpp"
#include <iostream>

namespace Phasor
{
Value VM::operation(const OpCode &op, const int &operand1, const int &operand2, const int &operand3,
                    const int &operand4, const int &operand5)
{
	uint8_t rA = static_cast<uint8_t>(operand1);
	uint8_t rB = static_cast<uint8_t>(operand2);
	uint8_t rC = static_cast<uint8_t>(operand3);
#ifdef _DEBUG
	log(std::string("OP: " + std::to_string(static_cast<int>(op)) + " operands=[" + std::to_string(operand1) + ", " + std::to_string(operand2) + ", " + std::to_string(operand3)
	          + ", " + std::to_string(operand4) + ", " + std::to_string(operand5) + "] stack=" + std::to_string(m_instance->activeFrame().stack.size()) + "\n"));
	flush();
#endif
	switch (op)
	{
	case OpCode::PUSH_CONST:
		if (operand1 < 0 || operand1 >= static_cast<int>(m_instance->code.constants.size()))
			throw std::runtime_error("Invalid constant index");
		push(m_instance->code.constants[operand1]);
		break;

	case OpCode::POP:
		pop();
		break;

	case OpCode::IADD: {
		Value b = pop();
		Value a = pop();
		push(asm_iadd(a.asInt(), b.asInt()));
		break;
	}

	case OpCode::ISUBTRACT: {
		Value b = pop();
		Value a = pop();
		push(asm_isub(a.asInt(), b.asInt()));
		break;
	}

	case OpCode::IMULTIPLY: {
		Value b = pop();
		Value a = pop();
		push(asm_imul(a.asInt(), b.asInt()));
		break;
	}

	case OpCode::IDIVIDE: {
		Value b = pop();
		Value a = pop();
		push(asm_idiv(a.asInt(), b.asInt()));
		break;
	}

	case OpCode::IMODULO: {
		Value b = pop();
		Value a = pop();
		push(asm_imod(a.asInt(), b.asInt()));
		break;
	}

	case OpCode::FLADD: {
		Value b = pop();
		Value a = pop();
		push(asm_fladd(a.asFloat(), b.asFloat()));
		break;
	}

	case OpCode::FLSUBTRACT: {
		Value b = pop();
		Value a = pop();
		push(asm_flsub(a.asFloat(), b.asFloat()));
		break;
	}

	case OpCode::FLMULTIPLY: {
		Value b = pop();
		Value a = pop();
		push(asm_flmul(a.asFloat(), b.asFloat()));
		break;
	}

	case OpCode::FLDIVIDE: {
		Value b = pop();
		Value a = pop();
		push(asm_fldiv(a.asFloat(), b.asFloat()));
		break;
	}

	case OpCode::FLMODULO: {
		Value b = pop();
		Value a = pop();
		push(asm_flmod(a.asFloat(), b.asFloat()));
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
		push(asm_flneg(pop().asFloat()));
		break;

	case OpCode::NOT:
		push(Value(asm_flnot(pop().isTruthy() ? 1 : 0)));
		break;

	case OpCode::IAND: {
		Value b = pop();
		Value a = pop();
		push(Value(asm_iand(a.isTruthy() ? 1 : 0, b.isTruthy() ? 1 : 0)));
		break;
	}

	case OpCode::IOR: {
		Value b = pop();
		Value a = pop();
		push(Value(asm_ior(a.isTruthy() ? 1 : 0, b.isTruthy() ? 1 : 0)));
		break;
	}

	case OpCode::IEQUAL: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_iequal(a.asInt(), b.asInt())) : Value(a == b));
		break;
	}

	case OpCode::INOT_EQUAL: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_inot_equal(a.asInt(), b.asInt())) : Value(a != b));
		break;
	}

	case OpCode::ILESS_THAN: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_iless_than(a.asInt(), b.asInt())) : Value(a < b));
		break;
	}

	case OpCode::IGREATER_THAN: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_igreater_than(a.asInt(), b.asInt())) : Value(a > b));
		break;
	}

	case OpCode::ILESS_EQUAL: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_iless_equal(a.asInt(), b.asInt())) : Value(a <= b));
		break;
	}

	case OpCode::IGREATER_EQUAL: {
		Value b = pop();
		Value a = pop();
		push(a.isInt() && b.isInt() ? Value(asm_igreater_equal(a.asInt(), b.asInt())) : Value(a >= b));
		break;
	}

	case OpCode::FLAND: {
		Value b = pop();
		Value a = pop();
		push(Value(asm_fland(a.isTruthy() ? 1 : 0, b.isTruthy() ? 1 : 0)));
		break;
	}

	case OpCode::FLOR: {
		Value b = pop();
		Value a = pop();
		push(Value(asm_flor(a.isTruthy() ? 1 : 0, b.isTruthy() ? 1 : 0)));
		break;
	}

	case OpCode::FLEQUAL: {
		Value b = pop();
		Value a = pop();
		push(a.isFloat() && b.isFloat() ? Value(asm_flequal(a.asFloat(), b.asFloat())) : Value(a == b));
		break;
	}

	case OpCode::FLNOT_EQUAL: {
		Value b = pop();
		Value a = pop();
		push(a.isFloat() && b.isFloat() ? Value(asm_flnot_equal(a.asFloat(), b.asFloat())) : Value(a != b));
		break;
	}

	case OpCode::FLLESS_THAN: {
		Value b = pop();
		Value a = pop();
		push(a.isFloat() && b.isFloat() ? Value(asm_flless_than(a.asFloat(), b.asFloat())) : Value(a < b));
		break;
	}

	case OpCode::FLGREATER_THAN: {
		Value b = pop();
		Value a = pop();
		push(a.isFloat() && b.isFloat() ? Value(asm_flgreater_than(a.asFloat(), b.asFloat())) : Value(a > b));
		break;
	}

	case OpCode::FLLESS_EQUAL: {
		Value b = pop();
		Value a = pop();
		push(a.isFloat() && b.isFloat() ? Value(asm_flless_equal(a.asFloat(), b.asFloat())) : Value(a <= b));
		break;
	}

	case OpCode::FLGREATER_EQUAL: {
		Value b = pop();
		Value a = pop();
		push(a.isFloat() && b.isFloat() ? Value(asm_flgreater_equal(a.asFloat(), b.asFloat())) : Value(a >= b));
		break;
	}

	case OpCode::JUMP:
		m_instance->activeFrame().pc = operand1;
		break;

	case OpCode::JUMP_IF_FALSE:
		if (!pop().isTruthy())
			m_instance->activeFrame().pc = operand1;
		break;

	case OpCode::JUMP_IF_TRUE:
		if (pop().isTruthy())
			m_instance->activeFrame().pc = operand1;
		break;

	case OpCode::JUMP_BACK:
		m_instance->activeFrame().pc = operand1;
		break;

	case OpCode::STORE_VAR:
		if (operand1 < 0 || operand1 >= static_cast<int>(m_instance->variables.size()))
			throw std::runtime_error("Invalid variable index");
		m_instance->variables[operand1] = pop();
		break;

	case OpCode::LOAD_VAR:
		if (operand1 < 0 || operand1 >= static_cast<int>(m_instance->variables.size()))
			throw std::runtime_error("Invalid variable index");
		push(m_instance->variables[operand1]);
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
			throw std::runtime_error("Import not implemented");
		break;
	}
	case OpCode::HALT:
		m_instance->alive = false;
		throw VM::Halt{};

	case OpCode::CALL_NATIVE: {
		Value       funcNameVal = m_instance->code.constants[operand1];
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
	case OpCode::TRUE_P:
		push(Value(true));
		break;

	case OpCode::FALSE_P:
		push(Value(false));
		break;

	case OpCode::NULL_VAL:
		push(Value());
		break;

	case OpCode::CALL: {
		std::string funcName = m_instance->code.constants[operand1].asString();

		auto it = m_instance->code.functionEntries.find(funcName);
		if (it == m_instance->code.functionEntries.end())
			throw std::runtime_error("Unknown function: " + funcName);

		int  paramCount = 0;
		auto pit = m_instance->code.functionParamCounts.find(funcName);
		if (pit != m_instance->code.functionParamCounts.end())
			paramCount = pit->second;

		std::vector<Value> args;
		args.reserve(paramCount);
		for (int i = 0; i < paramCount; ++i)
			args.push_back(pop());
		std::reverse(args.begin(), args.end());

		std::size_t returnPc = m_instance->activeFrame().pc;
		m_instance->pushFrame(NULL_HANDLE, returnPc);

		for (auto &arg : args)
			m_instance->activeFrame().stack.push_back(std::move(arg));

		m_instance->activeFrame().pc = static_cast<std::size_t>(it->second);
		break;
	}

	case OpCode::RETURN: {
		Value retVal;
		bool  hasRetVal = !m_instance->activeFrame().stack.empty();
		if (hasRetVal)
			retVal = pop();

		if (m_instance->callStack.size() == 1)
		{
			m_instance->popFrame();
			throw VM::Halt{};
		}

		std::size_t returnPc = m_instance->activeFrame().returnPc;
		m_instance->popFrame();
		m_instance->activeFrame().pc = returnPc;

		if (hasRetVal)
			push(retVal);

		break;
	}

	case OpCode::SYSTEM: {
		asm_system(m_instance->activeFrame().registers[rA].c_str());
		break;
	}

	case OpCode::SYSTEM_OUT: {
		push(asm_system_out(m_instance->activeFrame().registers[rA].c_str()));
		break;
	}

	case OpCode::SYSTEM_ERR: {
		push(asm_system_err(m_instance->activeFrame().registers[rA].c_str()));
		break;
	}

	// Register-based operations (v2.0)
	case OpCode::MOV: {
		m_instance->activeFrame().registers[rA] = m_instance->activeFrame().registers[rB];
		break;
	}

	case OpCode::LOAD_CONST_R: {
		// LOAD_CONST_R rA, constIndex
		int constIndex = operand2;
		if (constIndex < 0 || constIndex >= static_cast<int>(m_instance->code.constants.size()))
			throw std::runtime_error("Invalid constant index");
		m_instance->activeFrame().registers[rA] = m_instance->code.constants[constIndex];
		break;
	}

	case OpCode::LOAD_VAR_R: {
		// LOAD_VAR_R rA, varIndex
		int varIndex = operand2;
		if (varIndex < 0 || varIndex >= static_cast<int>(m_instance->variables.size()))
			throw std::runtime_error("Invalid variable index");
		m_instance->activeFrame().registers[rA] = m_instance->variables[varIndex];
		break;
	}

	case OpCode::STORE_VAR_R: {
		// STORE_VAR_R rA, varIndex - store register rA to variable varIndex
		int varIndex = operand2;
		if (varIndex < 0 || varIndex >= static_cast<int>(m_instance->variables.size()))
			throw std::runtime_error("Invalid variable index");
		m_instance->variables[varIndex] = m_instance->activeFrame().registers[rA];
		break;
	}

	// Register arithmetic (3-address code)
	// Format: operand1=rA (dest), operand2=rB, operand3=rC
	case OpCode::IADD_R: {
		m_instance->activeFrame().registers[rA] = Value(asm_iadd(m_instance->activeFrame().registers[rB].asInt(), m_instance->activeFrame().registers[rC].asInt()));
		break;
	}

	case OpCode::ISUB_R: {
		m_instance->activeFrame().registers[rA] = Value(asm_isub(m_instance->activeFrame().registers[rB].asInt(), m_instance->activeFrame().registers[rC].asInt()));
		break;
	}

	case OpCode::IMUL_R: {
		m_instance->activeFrame().registers[rA] = Value(asm_imul(m_instance->activeFrame().registers[rB].asInt(), m_instance->activeFrame().registers[rC].asInt()));
		break;
	}

	case OpCode::IDIV_R: {
		m_instance->activeFrame().registers[rA] = Value(asm_idiv(m_instance->activeFrame().registers[rB].asInt(), m_instance->activeFrame().registers[rC].asInt()));
		break;
	}

	case OpCode::IMOD_R: {
		m_instance->activeFrame().registers[rA] = Value(asm_imod(m_instance->activeFrame().registers[rB].asInt(), m_instance->activeFrame().registers[rC].asInt()));
		break;
	}

	case OpCode::FLADD_R: {
		m_instance->activeFrame().registers[rA] = Value(asm_fladd(m_instance->activeFrame().registers[rB].asFloat(), m_instance->activeFrame().registers[rC].asFloat()));
		break;
	}

	case OpCode::FLSUB_R: {
		m_instance->activeFrame().registers[rA] = Value(asm_flsub(m_instance->activeFrame().registers[rB].asFloat(), m_instance->activeFrame().registers[rC].asFloat()));
		break;
	}

	case OpCode::FLMUL_R: {
		m_instance->activeFrame().registers[rA] = Value(asm_flmul(m_instance->activeFrame().registers[rB].asFloat(), m_instance->activeFrame().registers[rC].asFloat()));
		break;
	}

	case OpCode::FLDIV_R: {
		m_instance->activeFrame().registers[rA] = Value(asm_fldiv(m_instance->activeFrame().registers[rB].asFloat(), m_instance->activeFrame().registers[rC].asFloat()));
		break;
	}

	case OpCode::FLMOD_R: {
		m_instance->activeFrame().registers[rA] = Value(asm_flmod(m_instance->activeFrame().registers[rB].asFloat(), m_instance->activeFrame().registers[rC].asFloat()));
		break;
	}

	case OpCode::SQRT_R: {
		m_instance->activeFrame().registers[rA] = Value(asm_sqrt(m_instance->activeFrame().registers[rB].asFloat()));
		break;
	}

	case OpCode::POW_R: {
		m_instance->activeFrame().registers[rA] = Value(asm_pow(m_instance->activeFrame().registers[rB].asFloat(), m_instance->activeFrame().registers[rC].asFloat()));
		break;
	}

	case OpCode::LOG_R: {
		m_instance->activeFrame().registers[rA] = Value(asm_log(m_instance->activeFrame().registers[rB].asFloat()));
		break;
	}

	case OpCode::EXP_R: {
		m_instance->activeFrame().registers[rA] = Value(asm_exp(m_instance->activeFrame().registers[rB].asFloat()));
		break;
	}

	case OpCode::SIN_R: {
		m_instance->activeFrame().registers[rA] = Value(asm_sin(m_instance->activeFrame().registers[rB].asFloat()));
		break;
	}

	case OpCode::COS_R: {
		m_instance->activeFrame().registers[rA] = Value(asm_cos(m_instance->activeFrame().registers[rB].asFloat()));
		break;
	}

	case OpCode::TAN_R: {
		m_instance->activeFrame().registers[rA] = Value(asm_tan(m_instance->activeFrame().registers[rB].asFloat()));
		break;
	}

	// Register comparisons
	// Format: operand1=rA (dest), operand2=rB, operand3=rC
	case OpCode::IEQ_R: {
		Value &b = m_instance->activeFrame().registers[rB];
		Value &c = m_instance->activeFrame().registers[rC];
		m_instance->activeFrame().registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_iequal(b.asInt(), c.asInt())) : Value(b == c);
		break;
	}

	case OpCode::INE_R: {
		Value &b = m_instance->activeFrame().registers[rB];
		Value &c = m_instance->activeFrame().registers[rC];
		m_instance->activeFrame().registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_inot_equal(b.asInt(), c.asInt())) : Value(b != c);
		break;
	}

	case OpCode::ILT_R: {
		Value &b = m_instance->activeFrame().registers[rB];
		Value &c = m_instance->activeFrame().registers[rC];
		m_instance->activeFrame().registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_iless_than(b.asInt(), c.asInt())) : Value(b < c);
		break;
	}

	case OpCode::IGT_R: {
		Value &b = m_instance->activeFrame().registers[rB];
		Value &c = m_instance->activeFrame().registers[rC];
		m_instance->activeFrame().registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_igreater_than(b.asInt(), c.asInt())) : Value(b > c);
		break;
	}

	case OpCode::ILE_R: {
		Value &b = m_instance->activeFrame().registers[rB];
		Value &c = m_instance->activeFrame().registers[rC];
		m_instance->activeFrame().registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_iless_equal(b.asInt(), c.asInt())) : Value(b <= c);
		break;
	}

	case OpCode::IGE_R: {
		Value &b = m_instance->activeFrame().registers[rB];
		Value &c = m_instance->activeFrame().registers[rC];
		m_instance->activeFrame().registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_igreater_equal(b.asInt(), c.asInt())) : Value(b >= c);
		break;
	}

	case OpCode::IAND_R: {
		Value &b = m_instance->activeFrame().registers[rB];
		Value &c = m_instance->activeFrame().registers[rC];
		m_instance->activeFrame().registers[rA] = Value(asm_iand(b.isTruthy() ? 1 : 0, c.isTruthy() ? 1 : 0));
		break;
	}

	case OpCode::IOR_R: {
		Value &b = m_instance->activeFrame().registers[rB];
		Value &c = m_instance->activeFrame().registers[rC];
		m_instance->activeFrame().registers[rA] = Value(asm_ior(b.isTruthy() ? 1 : 0, c.isTruthy() ? 1 : 0));
		break;
	}

	case OpCode::FLEQ_R: {
		Value &b = m_instance->activeFrame().registers[rB];
		Value &c = m_instance->activeFrame().registers[rC];
		m_instance->activeFrame().registers[rA] = (b.isFloat() && c.isFloat()) ? Value(asm_flequal(b.asFloat(), c.asFloat())) : Value(b == c);
		break;
	}

	case OpCode::FLNE_R: {
		Value &b = m_instance->activeFrame().registers[rB];
		Value &c = m_instance->activeFrame().registers[rC];
		m_instance->activeFrame().registers[rA] = (b.isFloat() && c.isFloat()) ? Value(asm_flnot_equal(b.asFloat(), c.asFloat())) : Value(b != c);
		break;
	}

	case OpCode::FLLT_R: {
		Value &b = m_instance->activeFrame().registers[rB];
		Value &c = m_instance->activeFrame().registers[rC];
		m_instance->activeFrame().registers[rA] = (b.isFloat() && c.isFloat()) ? Value(asm_flless_than(b.asFloat(), c.asFloat())) : Value(b < c);
		break;
	}

	case OpCode::FLGT_R: {
		Value &b = m_instance->activeFrame().registers[rB];
		Value &c = m_instance->activeFrame().registers[rC];
		m_instance->activeFrame().registers[rA] =
		    (b.isFloat() && c.isFloat()) ? Value(asm_flgreater_than(b.asFloat(), c.asFloat())) : Value(b > c);
		break;
	}

	case OpCode::FLLE_R: {
		Value &b = m_instance->activeFrame().registers[rB];
		Value &c = m_instance->activeFrame().registers[rC];
		m_instance->activeFrame().registers[rA] =
		    (b.isFloat() && c.isFloat()) ? Value(asm_flless_equal(b.asFloat(), c.asFloat())) : Value(b <= c);
		break;
	}

	case OpCode::FLGE_R: {
		Value &b = m_instance->activeFrame().registers[rB];
		Value &c = m_instance->activeFrame().registers[rC];
		m_instance->activeFrame().registers[rA] =
		    (b.isFloat() && c.isFloat()) ? Value(asm_flgreater_equal(b.asFloat(), c.asFloat())) : Value(b >= c);
		break;
	}

	case OpCode::FLAND_R: {
		Value &b = m_instance->activeFrame().registers[rB];
		Value &c = m_instance->activeFrame().registers[rC];
		m_instance->activeFrame().registers[rA] = Value(asm_fland(b.isTruthy() ? 1 : 0, c.isTruthy() ? 1 : 0));
		break;
	}

	case OpCode::FLOR_R: {
		Value &b = m_instance->activeFrame().registers[rB];
		Value &c = m_instance->activeFrame().registers[rC];
		m_instance->activeFrame().registers[rA] = Value(asm_flor(b.isTruthy() ? 1 : 0, c.isTruthy() ? 1 : 0));
		break;
	}

	// Register-stack interaction
	case OpCode::PUSH_R: {
		push(m_instance->activeFrame().registers[rA]);
		break;
	}

	case OpCode::PUSH2_R: {
		push(m_instance->activeFrame().registers[rA]);
		push(m_instance->activeFrame().registers[rB]);
		break;
	}

	case OpCode::POP_R: {
		m_instance->activeFrame().registers[rA] = pop();
		break;
	}

	case OpCode::POP2_R: {
		m_instance->activeFrame().registers[rA] = pop();
		m_instance->activeFrame().registers[rB] = pop();
		break;
	}

	// Register unary operations
	case OpCode::NEG_R: {
		// NEG_R rA, rB - rA = -rB
		m_instance->activeFrame().registers[rA] = Value(asm_flneg(m_instance->activeFrame().registers[rB].asFloat()));
		break;
	}

	case OpCode::NOT_R: {
		// NOT_R rA, rB - rA = !rB
		m_instance->activeFrame().registers[rA] = Value(asm_flnot(m_instance->activeFrame().registers[rB].isTruthy() ? 1 : 0));
		break;
	}

	// Register I/O
	case OpCode::PRINT_R: {
		std::string s = m_instance->activeFrame().registers[rA].toString();
		asm_print_stdout(s.c_str(), s.length());
		break;
	}

	case OpCode::PRINTERROR_R: {
		std::string s = m_instance->activeFrame().registers[rA].toString();
		asm_print_stderr(s.c_str(), s.length());
		break;
	}

	case OpCode::READLINE_R: {
		std::string s;
		std::getline(std::cin, s);
		m_instance->activeFrame().registers[rA] = s;
		break;
	}

	case OpCode::SYSTEM_R: {
		m_instance->activeFrame().registers[rA] = asm_system(m_instance->activeFrame().registers[rA].c_str());
		break;
	}

	case OpCode::SYSTEM_OUT_R: {
		m_instance->activeFrame().registers[rA] = asm_system_out(m_instance->activeFrame().registers[rA].c_str());
		break;
	}

	case OpCode::SYSTEM_ERR_R: {
		m_instance->activeFrame().registers[rA] = asm_system_err(m_instance->activeFrame().registers[rA].c_str());
		break;
	}

	case OpCode::LEN: {
		Value v = pop();
		push(Value(static_cast<int64_t>(v.asString().length())));
		break;
	}

	case OpCode::CHAR_AT: {
		Value idxVal = pop();
		Value strVal = pop();

		std::string s;
		if (strVal.isString())
			s = strVal.asString();
		else
			s = strVal.toString();

		int64_t idx = 0;
		if (idxVal.isInt())
			idx = idxVal.asInt();
		else if (idxVal.isFloat())
			idx = static_cast<int64_t>(idxVal.asFloat());
		else if (idxVal.isString())
		{
			// Try to parse numeric string
			try
			{
				idx = std::stoll(idxVal.asString());
			}
			catch (...)
			{
				throw std::runtime_error("char_at() expects index convertible to integer");
			}
		}
		else
			throw std::runtime_error("char_at() expects string and integer");

		if (idx < 0 || idx >= static_cast<int64_t>(s.length()))
			push(Value(""));
		else
			push(Value(std::string(1, s[static_cast<size_t>(idx)])));
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
		if (operand1 < 0 || operand1 >= static_cast<int>(m_instance->code.structs.size()))
			throw std::runtime_error("Invalid struct index for NEW_STRUCT_INSTANCE_STATIC");

		const StructInfo &info = m_instance->code.structs[operand1];
		// Create struct instance by name, then apply default values from constants
		Value instance = Value::createStruct(info.name);
		for (int i = 0; i < info.fieldCount; ++i)
		{
			int constIndex = info.firstConstIndex + i;
			if (constIndex < 0 || constIndex >= static_cast<int>(m_instance->code.constants.size()))
				throw std::runtime_error("Invalid default constant index for struct field");
			const Value       &defVal = m_instance->code.constants[constIndex];
			const std::string &fieldName = info.fieldNames[i];
			instance.setField(fieldName, defVal);
		}
		push(instance);
		break;
	}

	case OpCode::GET_FIELD_STATIC: {
		// operand1: structIndex, operand2: fieldOffset
		if (operand1 < 0 || operand1 >= static_cast<int>(m_instance->code.structs.size()))
			throw std::runtime_error("Invalid struct index for GET_FIELD_STATIC");
		const StructInfo &info = m_instance->code.structs[operand1];
		int               fieldOffset = operand2;
		if (fieldOffset < 0 || fieldOffset >= info.fieldCount)
			throw std::runtime_error("Invalid field offset for GET_FIELD_STATIC");
		const std::string &fieldName = info.fieldNames[fieldOffset];
		Value              obj = pop();
		push(obj.getField(fieldName));
		break;
	}

	case OpCode::SET_FIELD_STATIC: {
		// operand1: structIndex, operand2: fieldOffset
		if (operand1 < 0 || operand1 >= static_cast<int>(m_instance->code.structs.size()))
			throw std::runtime_error("Invalid struct index for SET_FIELD_STATIC");
		const StructInfo &info = m_instance->code.structs[operand1];
		int               fieldOffset = operand2;
		if (fieldOffset < 0 || fieldOffset >= info.fieldCount)
			throw std::runtime_error("Invalid field offset for SET_FIELD_STATIC");
		const std::string &fieldName = info.fieldNames[fieldOffset];
		Value              value = pop();
		Value              obj = pop();
		obj.setField(fieldName, value);
		push(obj);
		break;
	}

	case OpCode::NEW_STRUCT: {
		if (operand1 < 0 || operand1 >= static_cast<int>(m_instance->code.constants.size()))
			throw std::runtime_error("Invalid constant index for NEW_STRUCT");
		Value       nameVal = m_instance->code.constants[operand1];
		std::string structName = nameVal.asString();
		push(Value::createStruct(structName));
		break;
	}

	case OpCode::SET_FIELD: {
		if (operand1 < 0 || operand1 >= static_cast<int>(m_instance->code.constants.size()))
			throw std::runtime_error("Invalid constant index for SET_FIELD");
		std::string fieldName = m_instance->code.constants[operand1].asString();
		Value       value = pop();
		Value       obj = pop();
		obj.setField(fieldName, value);
		push(obj);
		break;
	}

	case OpCode::GET_FIELD: {
		if (operand1 < 0 || operand1 >= static_cast<int>(m_instance->code.constants.size()))
			throw std::runtime_error("Invalid constant index for GET_FIELD");
		std::string fieldName = m_instance->code.constants[operand1].asString();
		Value       obj = pop();
		push(obj.getField(fieldName));
		break;
	}

	default:
		throw std::runtime_error("Unknown opcode");
	}
	return Value(operand1);
}
} // namespace Phasor