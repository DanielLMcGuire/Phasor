#ifndef OPS_ARE_INCLUDED
#include "VM.hpp" // avoid breaking IDEs
#endif

namespace Phasor {

Value VM::operation(const OpCode &op, const int &operand1, const int &operand2, const int &operand3)
{
	uint8_t rA = static_cast<uint8_t>(operand1);
	uint8_t rB = static_cast<uint8_t>(operand2);
	uint8_t rC = static_cast<uint8_t>(operand3);
#ifdef TRACING
	log(std::format("VM::{}({}, {}, {}, {})\n", __func__, opCodeToString(op), operand1, operand2, operand3));
	flush();
#endif
	switch (op)
	{

	#pragma region CONTROL FLOW

	[[likely]] case OpCode::JUMP: {
#ifdef TRACING
		log(std::format("JUMP: {} -> {}\n", pc, operand1));
		flush();
#endif
		pc = operand1;
		break;
	}

	[[likely]] case OpCode::CALL: {
		Value       funcNameVal = m_bytecode->constants[operand1];
		std::string funcName = funcNameVal.asString();
		auto        it = m_bytecode->functionEntries.find(funcName);
		if (it == m_bytecode->functionEntries.end())
			throw std::runtime_error("Unknown function: " + funcName);
#ifdef TRACING
		log(std::format("CALL: {} -> {}: {}\n", pc, funcName, it->second));
		flush();
#endif
		callStack.push_back(static_cast<int>(pc));
		pc = it->second;
		break;
	}
	[[likely]] case OpCode::RETURN: {
		if (callStack.empty()) [[unlikely]]
		{
			pc = m_bytecode->instructions.size();
			throw std::runtime_error("Cannot return from outside a function");
			break;
		}
#ifdef TRACING
		log(std::format("RETURN: {} -> {}\n", pc, callStack.back()));
		flush();
#endif
		pc = callStack.back();
		callStack.pop_back();
		break;
	}

	[[likely]] case OpCode::CALL_NATIVE: {
		Value       funcNameVal = m_bytecode->constants[operand1];
		std::string funcName = funcNameVal.asString();
		auto        it = nativeFunctions.find(funcName);
		if (it == nativeFunctions.end())
			throw std::runtime_error("Unknown native function: " + funcName);

		int                argCount = static_cast<int>(pop().asInt());
		std::vector<Value> args(argCount);
		for (int i = argCount - 1; i >= 0; --i)
			args[i] = pop();

#ifdef TRACING
		std::string argsText;
		for (auto &arg : args)
		{
			argsText += std::format("{:T}", arg);
			if (arg != args.back())
				argsText += ", ";
		}
		log(std::format("CALL_NATIVE: {}({})\n", funcName, argsText));
		flush();
#endif

		push(it->second(args, this));

		break;
	}

	case OpCode::JUMP_IF_FALSE: {
#ifdef TRACING
		log(std::format("JUMP_IF_FALSE: {} {} -> {}\n", peek().isTruthy() ? "TRUE" : "FALSE", pc, operand1));
		flush();
#endif
		if (!pop().isTruthy()) pc = operand1;
		break;
	}

	case OpCode::JUMP_IF_TRUE: {
#ifdef TRACING
		log(std::format("JUMP_IF_TRUE: {} {} -> {}\n", peek().isTruthy() ? "TRUE" : "FALSE", pc, operand1));
		flush();
#endif
		if (pop().isTruthy()) pc = operand1;
		break;
	}

	case OpCode::JUMP_BACK: {
#ifdef TRACING
		log(std::format("JUMP_BACK: {} -> {}\n", pc, operand1));
		flush();
#endif
		pc = operand1;
		break;
	}

	[[unlikely]] case OpCode::IMPORT: {
		Value       pathVal = m_bytecode->constants[operand1];
		std::string path = pathVal.asString();
		if (importHandler)
			importHandler(path);
		else
			throw std::runtime_error("Import handler not set");
		break;
	}
	
	[[unlikely]] case OpCode::HALT: {
		pc = m_bytecode->instructions.size();
		throw VM::Halt();
		break;
	}

	#pragma endregion
	#pragma region STACK CORE

	[[likely]] case OpCode::PUSH_CONST: {
		if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->constants.size()))
			throw std::runtime_error("Invalid constant index");
		push(m_bytecode->constants[operand1]);
		break;
	}

	[[likely]] case OpCode::POP: {
		pop();
		break;
	}

	[[likely]] case OpCode::STORE_VAR: {
		if (operand1 < 0 || operand1 >= static_cast<int>(variables.size()))
			throw std::runtime_error("Invalid variable index");
		variables[operand1] = pop();
		break;
	}

	[[likely]] case OpCode::LOAD_VAR: {
		if (operand1 < 0 || operand1 >= static_cast<int>(variables.size()))
			throw std::runtime_error("Invalid variable index");
		push(variables[operand1]);
		break;
	}

	case OpCode::TRUE_P: {
		push(Value(true));
		break;
	}

	case OpCode::FALSE_P: {
		push(Value(false));
		break;
	}

	case OpCode::NULL_VAL: {
		push(Value());
		break;
	}

	#pragma endregion
	#pragma region STACK ARITHMETIC

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

	#pragma endregion
	#pragma region STACK LOGICAL

	case OpCode::NEGATE: {
		push(asm_flneg(pop().asFloat()));
		break;
	}

	case OpCode::NOT: {
		push(Value(asm_flnot(pop().isTruthy() ? 1 : 0)));
		break;
	}

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

	#pragma endregion
	#pragma region STACK I/O

	case OpCode::PRINT: {
		Value       v = pop();
		std::string s = v.toString();
#ifdef TRACING
		log(std::format("PRINT: (stdout) {:T}\n", v));
#else
		c_print_stdout(s.c_str(), (int64_t)s.length());
#endif
		break;
	}

	[[unlikely]] case OpCode::PRINTERROR: {
		Value       v = pop();
		std::string s = v.toString();
#ifdef TRACING
		log(std::format("PRINTERROR: (stderr) {:T}\n", v));
#else
		c_print_stderr(s.c_str(), (int64_t)s.length());
#endif
		flusherr();
		break;
	}

	case OpCode::READLINE: {
		std::string s;
#ifdef TRACING
		log("READLINE:");
		flush();
#endif
		std::getline(std::cin, s);
#ifdef TRACING
		log(std::format("\nREADLINE: {}\n", s));
#endif
		push(s);
		break;
	}

	#pragma endregion
	#pragma region STACK SYSTEM

	case OpCode::SYSTEM: {
#ifdef SANDBOXED
		logerr("CANNOT ESCAPE SANDBOX");
		push(Value());
#else
	#ifdef TRACING
			Value cmd = pop();
			int ret = c_system(cmd.c_str());
			log(std::format("SYSTEM: {:T} -> {}\n", cmd, ret));
			push(ret);
	#else
			push(c_system(pop().c_str()));
	#endif
#endif
		break;
	}

	case OpCode::SYSTEM_OUT: {
#ifdef SANDBOXED
		logerr("CANNOT ESCAPE SANDBOX");
		push(Value());
#else
	#ifdef TRACING
			Value cmd = pop();
			std::string ret = c_system_out(cmd.c_str());
			log(std::format("SYSTEM_OUT: {:T} -> {}\n", cmd, ret));
			push(ret);
	#else
			push(c_system_out(pop().c_str()));
	#endif
#endif
		break;
	}

	case OpCode::SYSTEM_ERR: {
#ifdef SANDBOXED
		logerr("CANNOT ESCAPE SANDBOX");
		push(Value());
#else
	#ifdef TRACING
			Value cmd = pop();
			std::string ret = c_system_err(cmd.c_str());
			log(std::format("SYSTEM_ERR: {:T} -> {}\n", cmd, ret));
			push(ret);
	#else
			push(c_system_err(pop().c_str()));
	#endif
#endif
		break;
	}

	#pragma endregion
	#pragma region STACK STRING

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

	#pragma endregion
	#pragma region STACK STRUCT

	case OpCode::NEW_STRUCT_INSTANCE_STATIC: {
		if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->structs.size()))
			throw std::runtime_error("Invalid struct index for NEW_STRUCT_INSTANCE_STATIC");

		const StructInfo &info = m_bytecode->structs[operand1];
		Value instance = Value::createStruct(info.name);
		for (int i = 0; i < info.fieldCount; ++i)
		{
			int constIndex = info.firstConstIndex + i;
			if (constIndex < 0 || constIndex >= static_cast<int>(m_bytecode->constants.size()))
				throw std::runtime_error("Invalid default constant index for struct field");
			const Value       &defVal = m_bytecode->constants[constIndex];
			const std::string &fieldName = info.fieldNames[i];
			instance.setField(fieldName, defVal);
		}
		push(instance);
		break;
	}

	case OpCode::GET_FIELD_STATIC: {
		if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->structs.size()))
			throw std::runtime_error("Invalid struct index for GET_FIELD_STATIC");
		const StructInfo &info = m_bytecode->structs[operand1];
		int fieldOffset = operand2;
		if (fieldOffset < 0 || fieldOffset >= info.fieldCount)
			throw std::runtime_error("Invalid field offset for GET_FIELD_STATIC");
		const std::string &fieldName = info.fieldNames[fieldOffset];
		Value              obj = pop();
		push(obj.getField(fieldName));
		break;
	}

	case OpCode::SET_FIELD_STATIC: {
		if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->structs.size()))
			throw std::runtime_error("Invalid struct index for SET_FIELD_STATIC");
		const StructInfo &info = m_bytecode->structs[operand1];
		int fieldOffset = operand2;
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
		if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->constants.size()))
			throw std::runtime_error("Invalid constant index for NEW_STRUCT");
		Value       nameVal = m_bytecode->constants[operand1];
		std::string structName = nameVal.asString();
		push(Value::createStruct(structName));
		break;
	}

	case OpCode::SET_FIELD: {
		if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->constants.size()))
			throw std::runtime_error("Invalid constant index for SET_FIELD");
		std::string fieldName = m_bytecode->constants[operand1].asString();
		Value       value = pop();
		Value       obj = pop();
		obj.setField(fieldName, value);
		push(obj);
		break;
	}

	case OpCode::GET_FIELD: {
		if (operand1 < 0 || operand1 >= static_cast<int>(m_bytecode->constants.size()))
			throw std::runtime_error("Invalid constant index for GET_FIELD");
		std::string fieldName = m_bytecode->constants[operand1].asString();
		Value       obj = pop();
		push(obj.getField(fieldName));
		break;
	}

	#pragma endregion

	#pragma region REGISTER CORE

	[[likely]] case OpCode::MOV: {
		registers[rA] = registers[rB];
		break;
	}

	[[likely]] case OpCode::LOAD_CONST_R: {
		int constIndex = operand2;
		if (constIndex < 0 || constIndex >= static_cast<int>(m_bytecode->constants.size()))
			throw std::runtime_error("Invalid constant index");
		registers[rA] = m_bytecode->constants[constIndex];
		break;
	}

	[[likely]] case OpCode::LOAD_VAR_R: {
		int varIndex = operand2;
		if (varIndex < 0 || varIndex >= static_cast<int>(variables.size()))
			throw std::runtime_error("Invalid variable index");
		registers[rA] = variables[varIndex];
		break;
	}

	[[likely]] case OpCode::STORE_VAR_R: {
		int varIndex = operand2;
		if (varIndex < 0 || varIndex >= static_cast<int>(variables.size()))
			throw std::runtime_error("Invalid variable index");
		variables[varIndex] = registers[rA];
		break;
	}

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

	#pragma region REG ARITHMETIC

	case OpCode::IADD_R: {
		registers[rA] = Value(asm_iadd(registers[rB].asInt(), registers[rC].asInt()));
		break;
	}

	case OpCode::ISUB_R: {
		registers[rA] = Value(asm_isub(registers[rB].asInt(), registers[rC].asInt()));
		break;
	}

	case OpCode::IMUL_R: {
		registers[rA] = Value(asm_imul(registers[rB].asInt(), registers[rC].asInt()));
		break;
	}

	case OpCode::IDIV_R: {
		registers[rA] = Value(asm_idiv(registers[rB].asInt(), registers[rC].asInt()));
		break;
	}

	case OpCode::IMOD_R: {
		registers[rA] = Value(asm_imod(registers[rB].asInt(), registers[rC].asInt()));
		break;
	}

	case OpCode::FLADD_R: {
		registers[rA] = Value(asm_fladd(registers[rB].asFloat(), registers[rC].asFloat()));
		break;
	}

	case OpCode::FLSUB_R: {
		registers[rA] = Value(asm_flsub(registers[rB].asFloat(), registers[rC].asFloat()));
		break;
	}

	case OpCode::FLMUL_R: {
		registers[rA] = Value(asm_flmul(registers[rB].asFloat(), registers[rC].asFloat()));
		break;
	}

	case OpCode::FLDIV_R: {
		registers[rA] = Value(asm_fldiv(registers[rB].asFloat(), registers[rC].asFloat()));
		break;
	}

	case OpCode::FLMOD_R: {
		registers[rA] = Value(asm_flmod(registers[rB].asFloat(), registers[rC].asFloat()));
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

	#pragma endregion
	#pragma region REG LOGICAL

	case OpCode::NEG_R: {
		registers[rA] = Value(asm_flneg(registers[rB].asFloat()));
		break;
	}

	case OpCode::NOT_R: {
		registers[rA] = Value(asm_flnot(registers[rB].isTruthy() ? 1 : 0));
		break;
	}

	case OpCode::IEQ_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_iequal(b.asInt(), c.asInt())) : Value(b == c);
		break;
	}

	case OpCode::INE_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_inot_equal(b.asInt(), c.asInt())) : Value(b != c);
		break;
	}

	case OpCode::ILT_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_iless_than(b.asInt(), c.asInt())) : Value(b < c);
		break;
	}

	case OpCode::IGT_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_igreater_than(b.asInt(), c.asInt())) : Value(b > c);
		break;
	}

	case OpCode::ILE_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_iless_equal(b.asInt(), c.asInt())) : Value(b <= c);
		break;
	}

	case OpCode::IGE_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isInt() && c.isInt()) ? Value(asm_igreater_equal(b.asInt(), c.asInt())) : Value(b >= c);
		break;
	}

	case OpCode::IAND_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = Value(asm_iand(b.isTruthy() ? 1 : 0, c.isTruthy() ? 1 : 0));
		break;
	}

	case OpCode::IOR_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = Value(asm_ior(b.isTruthy() ? 1 : 0, c.isTruthy() ? 1 : 0));
		break;
	}

	case OpCode::FLEQ_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isFloat() && c.isFloat()) ? Value(asm_flequal(b.asFloat(), c.asFloat())) : Value(b == c);
		break;
	}

	case OpCode::FLNE_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isFloat() && c.isFloat()) ? Value(asm_flnot_equal(b.asFloat(), c.asFloat())) : Value(b != c);
		break;
	}

	case OpCode::FLLT_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = (b.isFloat() && c.isFloat()) ? Value(asm_flless_than(b.asFloat(), c.asFloat())) : Value(b < c);
		break;
	}

	case OpCode::FLGT_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] =
		    (b.isFloat() && c.isFloat()) ? Value(asm_flgreater_than(b.asFloat(), c.asFloat())) : Value(b > c);
		break;
	}

	case OpCode::FLLE_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] =
		    (b.isFloat() && c.isFloat()) ? Value(asm_flless_equal(b.asFloat(), c.asFloat())) : Value(b <= c);
		break;
	}

	case OpCode::FLGE_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] =
		    (b.isFloat() && c.isFloat()) ? Value(asm_flgreater_equal(b.asFloat(), c.asFloat())) : Value(b >= c);
		break;
	}

	case OpCode::FLAND_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = Value(asm_fland(b.isTruthy() ? 1 : 0, c.isTruthy() ? 1 : 0));
		break;
	}

	case OpCode::FLOR_R: {
		Value &b = registers[rB];
		Value &c = registers[rC];
		registers[rA] = Value(asm_flor(b.isTruthy() ? 1 : 0, c.isTruthy() ? 1 : 0));
		break;
	}

	#pragma endregion
	#pragma region REG I/O

	case OpCode::PRINT_R: {
		std::string s = registers[rA].toString();
#ifdef TRACING
		log(std::format("PRINT_R: (stdout) {:T}\n", registers[rA]));
#else
		c_print_stdout(s.c_str(), (int64_t)s.length());
#endif
		break;
	}

	[[unlikely]] case OpCode::PRINTERROR_R: {
		std::string s = registers[rA].toString();
#ifdef TRACING
		log(std::format("PRINTERROR_R: (stderr) {:T}\n", registers[rA]));
#else
		c_print_stderr(s.c_str(), (int64_t)s.length());
#endif
		flusherr();
		break;
	}

	case OpCode::READLINE_R: {
		std::string s;
#ifdef TRACING
		log("READLINE_R:");
		flush();
#endif
		std::getline(std::cin, s);
#ifdef TRACING
		log(std::format("\nREADLINE_R: {}\n", s));
#endif
		registers[rA] = s;
		break;
	}

	#pragma endregion
	#pragma region REG SYSTEM

	case OpCode::SYSTEM_R: {
#ifdef SANDBOXED
		logerr("CANNOT ESCAPE SANDBOX");
		registers[rA] = Value();
#else
	#ifdef TRACING
			Value cmd = registers[rA];
			int64_t ret = c_system(cmd.c_str());
			log(std::format("SYSTEM_R: {} -> {}\n", cmd, ret));
			registers[rA] = ret;
	#else
			registers[rA] = c_system(registers[rA].c_str());
	#endif
#endif
		break;
	}

	case OpCode::SYSTEM_OUT_R: {
#ifdef SANDBOXED
		logerr("CANNOT ESCAPE SANDBOX");
		registers[rA] = Value();
#else
	#ifdef TRACING
			Value cmd = registers[rA];
			std::string ret = c_system_out(cmd.c_str());
			log(std::format("SYSTEM_R: {:T} -> {}\n", cmd, ret));
			registers[rA] = ret;
	#else
			registers[rA] = c_system_out(registers[rA].c_str());
	#endif
#endif
		break;
	}

	case OpCode::SYSTEM_ERR_R: {
#ifdef SANDBOXED
		logerr("CANNOT ESCAPE SANDBOX");
		registers[rA] = Value();
#else
	#ifdef TRACING
			Value cmd = registers[rA];
			std::string ret = c_system_err(cmd.c_str());
			log(std::format("SYSTEM_ERR_R: {:T} -> {}\n", cmd, ret));
			registers[rA] = ret;
	#else
			registers[rA] = c_system_err(registers[rA].c_str());
	#endif
#endif
		break;
	}

	#pragma endregion

#pragma endregion

#pragma region DEFAULT
	default: {
		throw std::runtime_error("Unknown opcode");
		return Value();
	}
#pragma endregion
	
	}
	return Value(operand1);
}

} // namespace Phasor