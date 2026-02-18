#include "PhasorIR.hpp"
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

#if defined(_MSC_VER)
#define COMPILE_MESSAGE(msg) __pragma(message(msg))
#elif defined(__GNUC__) || defined(__clang__)
#define DO_PRAGMA(x) _Pragma(#x)
#define COMPILE_MESSAGE(msg) DO_PRAGMA(message msg)
#else
#define COMPILE_MESSAGE(msg)
#endif
#define STR2(x) #x
#define STR(x) STR2(x)

namespace Phasor
{

const std::unordered_map<OpCode, std::string> PhasorIR::opCodeToStringMap = {
    {OpCode::PUSH_CONST, "PUSH_CONST"},
    {OpCode::POP, "POP"},
    {OpCode::IADD, "IADD"},
    {OpCode::ISUBTRACT, "ISUBTRACT"},
    {OpCode::IMULTIPLY, "IMULTIPLY"},
    {OpCode::IDIVIDE, "IDIVIDE"},
    {OpCode::IMODULO, "IMODULO"},
    {OpCode::FLADD, "FLADD"},
    {OpCode::FLSUBTRACT, "FLSUBTRACT"},
    {OpCode::FLMULTIPLY, "FLMULTIPLY"},
    {OpCode::FLDIVIDE, "FLDIVIDE"},
    {OpCode::FLMODULO, "FLMODULO"},
    {OpCode::SQRT, "SQRT"},
    {OpCode::POW, "POW"},
    {OpCode::LOG, "LOG"},
    {OpCode::EXP, "EXP"},
    {OpCode::SIN, "SIN"},
    {OpCode::COS, "COS"},
    {OpCode::TAN, "TAN"},
    {OpCode::NEGATE, "NEGATE"},
    {OpCode::NOT, "NOT"},
    {OpCode::IAND, "IAND"},
    {OpCode::IOR, "IOR"},
    {OpCode::IEQUAL, "IEQUAL"},
    {OpCode::INOT_EQUAL, "INOT_EQUAL"},
    {OpCode::ILESS_THAN, "ILESS_THAN"},
    {OpCode::IGREATER_THAN, "IGREATER_THAN"},
    {OpCode::ILESS_EQUAL, "ILESS_EQUAL"},
    {OpCode::IGREATER_EQUAL, "IGREATER_EQUAL"},
    {OpCode::FLAND, "FLAND"},
    {OpCode::FLOR, "FLOR"},
    {OpCode::FLEQUAL, "FLEQUAL"},
    {OpCode::FLNOT_EQUAL, "FLNOT_EQUAL"},
    {OpCode::FLLESS_THAN, "FLLESS_THAN"},
    {OpCode::FLGREATER_THAN, "FLGREATER_THAN"},
    {OpCode::FLLESS_EQUAL, "FLLESS_EQUAL"},
    {OpCode::FLGREATER_EQUAL, "FLGREATER_EQUAL"},
    {OpCode::JUMP, "JUMP"},
    {OpCode::JUMP_IF_FALSE, "JUMP_IF_FALSE"},
    {OpCode::JUMP_IF_TRUE, "JUMP_IF_TRUE"},
    {OpCode::JUMP_BACK, "JUMP_BACK"},
    {OpCode::STORE_VAR, "STORE_VAR"},
    {OpCode::LOAD_VAR, "LOAD_VAR"},
    {OpCode::PRINT, "PRINT"},
    {OpCode::PRINTERROR, "PRINTERROR"},
    {OpCode::READLINE, "READLINE"},
    {OpCode::IMPORT, "IMPORT"},
    {OpCode::HALT, "HALT"},
    {OpCode::CALL_NATIVE, "CALL_NATIVE"},
    {OpCode::CALL, "CALL"},
    {OpCode::SYSTEM, "SYSTEM"},
    {OpCode::SYSTEM_OUT, "SYSTEM_OUT"},
    {OpCode::SYSTEM_ERR, "SYSTEM_ERR"},
    {OpCode::RETURN, "RETURN"},
    {OpCode::TRUE_P, "TRUE"},
    {OpCode::FALSE_P, "FALSE"},
    {OpCode::NULL_VAL, "NULL_VAL"},
    {OpCode::LEN, "LEN"},
    {OpCode::CHAR_AT, "CHAR_AT"},
    {OpCode::SUBSTR, "SUBSTR"},
    {OpCode::MOV, "MOV"},
    {OpCode::LOAD_CONST_R, "LOAD_CONST_R"},
    {OpCode::LOAD_VAR_R, "LOAD_VAR_R"},
    {OpCode::STORE_VAR_R, "STORE_VAR_R"},
    {OpCode::IADD_R, "IADD_R"},
    {OpCode::ISUB_R, "ISUB_R"},
    {OpCode::IMUL_R, "IMUL_R"},
    {OpCode::IDIV_R, "IDIV_R"},
    {OpCode::IMOD_R, "IMOD_R"},
    {OpCode::FLADD_R, "FLADD_R"},
    {OpCode::FLSUB_R, "FLSUB_R"},
    {OpCode::FLMUL_R, "FLMUL_R"},
    {OpCode::FLDIV_R, "FLDIV_R"},
    {OpCode::FLMOD_R, "FLMOD_R"},
    {OpCode::SQRT_R, "SQRT_R"},
    {OpCode::POW_R, "POW_R"},
    {OpCode::LOG_R, "LOG_R"},
    {OpCode::EXP_R, "EXP_R"},
    {OpCode::SIN_R, "SIN_R"},
    {OpCode::COS_R, "COS_R"},
    {OpCode::TAN_R, "TAN_R"},
    {OpCode::IAND_R, "IAND_R"},
    {OpCode::IOR_R, "IOR_R"},
    {OpCode::IEQ_R, "IEQ_R"},
    {OpCode::INE_R, "INE_R"},
    {OpCode::ILT_R, "ILT_R"},
    {OpCode::IGT_R, "IGT_R"},
    {OpCode::ILE_R, "ILE_R"},
    {OpCode::IGE_R, "IGE_R"},
    {OpCode::FLAND_R, "FLAND_R"},
    {OpCode::FLOR_R, "FLOR_R"},
    {OpCode::FLEQ_R, "FLEQ_R"},
    {OpCode::FLNE_R, "FLNE_R"},
    {OpCode::FLLT_R, "FLLT_R"},
    {OpCode::FLGT_R, "FLGT_R"},
    {OpCode::FLLE_R, "FLLE_R"},
    {OpCode::FLGE_R, "FLGE_R"},
    {OpCode::PUSH_R, "PUSH_R"},
    {OpCode::PUSH2_R, "PUSH2_R"},
    {OpCode::POP_R, "POP_R"},
    {OpCode::POP2_R, "POP2_R"},
    {OpCode::NEG_R, "NEG_R"},
    {OpCode::NOT_R, "NOT_R"},
    {OpCode::PRINT_R, "PRINT_R"},
    {OpCode::PRINTERROR_R, "PRINTERROR_R"},
    {OpCode::READLINE_R, "READLINE_R"},
    {OpCode::SYSTEM_R, "SYSTEM_R"},
    {OpCode::SYSTEM_OUT_R, "SYSTEM_OUT_R"},
    {OpCode::SYSTEM_ERR_R, "SYSTEM_ERR_R"}};

const std::unordered_map<std::string, OpCode> PhasorIR::stringToOpCodeMap = [] {
	std::unordered_map<std::string, OpCode> map;
	for (const auto &pair : PhasorIR::opCodeToStringMap)
	{
		map[pair.second] = pair.first;
	}
	return map;
}();

int PhasorIR::getOperandCount(OpCode op)
{
	switch (op)
	{
	// 0 operands
	case OpCode::POP:
	case OpCode::IADD:
	case OpCode::ISUBTRACT:
	case OpCode::IMULTIPLY:
	case OpCode::IDIVIDE:
	case OpCode::IMODULO:
	case OpCode::FLADD:
	case OpCode::FLSUBTRACT:
	case OpCode::FLMULTIPLY:
	case OpCode::FLDIVIDE:
	case OpCode::FLMODULO:
	case OpCode::SQRT:
	case OpCode::POW:
	case OpCode::LOG:
	case OpCode::EXP:
	case OpCode::SIN:
	case OpCode::COS:
	case OpCode::TAN:
	case OpCode::NEGATE:
	case OpCode::NOT:
	case OpCode::IAND:
	case OpCode::IOR:
	case OpCode::IEQUAL:
	case OpCode::INOT_EQUAL:
	case OpCode::ILESS_THAN:
	case OpCode::IGREATER_THAN:
	case OpCode::ILESS_EQUAL:
	case OpCode::IGREATER_EQUAL:
	case OpCode::FLEQUAL:
	case OpCode::FLNOT_EQUAL:
	case OpCode::FLLESS_THAN:
	case OpCode::FLGREATER_THAN:
	case OpCode::FLLESS_EQUAL:
	case OpCode::FLGREATER_EQUAL:
	case OpCode::PRINT:
	case OpCode::PRINTERROR:
	case OpCode::READLINE:
	case OpCode::HALT:
	case OpCode::RETURN:
	case OpCode::TRUE_P:
	case OpCode::FALSE_P:
	case OpCode::NULL_VAL:
	case OpCode::LEN:
	case OpCode::CHAR_AT:
	case OpCode::SUBSTR:
		return 0;

	// 1 operand
	case OpCode::PUSH_CONST:
	case OpCode::JUMP:
	case OpCode::JUMP_IF_FALSE:
	case OpCode::JUMP_IF_TRUE:
	case OpCode::JUMP_BACK:
	case OpCode::STORE_VAR:
	case OpCode::LOAD_VAR:
	case OpCode::IMPORT:
	case OpCode::CALL_NATIVE:
	case OpCode::CALL:
	case OpCode::SYSTEM:
	case OpCode::SYSTEM_OUT:
	case OpCode::SYSTEM_ERR:
	case OpCode::PUSH_R:
	case OpCode::POP_R:
	case OpCode::PRINT_R:
	case OpCode::PRINTERROR_R:
	case OpCode::READLINE_R:
	case OpCode::SYSTEM_R:
	case OpCode::SYSTEM_OUT_R:
	case OpCode::SYSTEM_ERR_R:
	case OpCode::NEW_STRUCT:
	case OpCode::GET_FIELD:
	case OpCode::SET_FIELD:
	case OpCode::NEW_STRUCT_INSTANCE_STATIC:
		return 1;

	// 2 operands
	case OpCode::MOV:
	case OpCode::LOAD_CONST_R:
	case OpCode::LOAD_VAR_R:
	case OpCode::STORE_VAR_R:
	case OpCode::SQRT_R:
	case OpCode::LOG_R:
	case OpCode::EXP_R:
	case OpCode::SIN_R:
	case OpCode::COS_R:
	case OpCode::TAN_R:
	case OpCode::NEG_R:
	case OpCode::NOT_R:
	case OpCode::PUSH2_R:
	case OpCode::POP2_R:
	case OpCode::GET_FIELD_STATIC:
	case OpCode::SET_FIELD_STATIC:
		return 2;

	// 3 operands
	case OpCode::IADD_R:
	case OpCode::ISUB_R:
	case OpCode::IMUL_R:
	case OpCode::IDIV_R:
	case OpCode::IMOD_R:
	case OpCode::FLADD_R:
	case OpCode::FLSUB_R:
	case OpCode::FLMUL_R:
	case OpCode::FLDIV_R:
	case OpCode::FLMOD_R:
	case OpCode::POW_R:
	case OpCode::IAND_R:
	case OpCode::IOR_R:
	case OpCode::IEQ_R:
	case OpCode::INE_R:
	case OpCode::ILT_R:
	case OpCode::IGT_R:
	case OpCode::ILE_R:
	case OpCode::IGE_R:
	case OpCode::FLAND_R:
	case OpCode::FLOR_R:
	case OpCode::FLEQ_R:
	case OpCode::FLNE_R:
	case OpCode::FLLT_R:
	case OpCode::FLGT_R:
	case OpCode::FLLE_R:
	case OpCode::FLGE_R:
		return 3;

	default:
		return 0;
	}
}

PhasorIR::OperandType PhasorIR::getOperandType(OpCode op, int operandIndex)
{
	// Stack operations with special indices
	if (op == OpCode::PUSH_CONST && operandIndex == 0)
		return OperandType::CONSTANT_IDX;
	if (op == OpCode::STORE_VAR && operandIndex == 0)
		return OperandType::VARIABLE_IDX;
	if (op == OpCode::LOAD_VAR && operandIndex == 0)
		return OperandType::VARIABLE_IDX;
	if (op == OpCode::IMPORT && operandIndex == 0)
		return OperandType::CONSTANT_IDX;
	if (op == OpCode::CALL_NATIVE && operandIndex == 0)
		return OperandType::CONSTANT_IDX;
	if (op == OpCode::CALL && operandIndex == 0)
		return OperandType::FUNCTION_IDX;
	if (op == OpCode::SYSTEM && operandIndex == 0)
		return OperandType::CONSTANT_IDX;

	// Register operations with mixed types
	if (op == OpCode::LOAD_CONST_R)
	{
		if (operandIndex == 0)
			return OperandType::REGISTER;
		if (operandIndex == 1)
			return OperandType::CONSTANT_IDX;
	}
	if (op == OpCode::LOAD_VAR_R)
	{
		if (operandIndex == 0)
			return OperandType::REGISTER;
		if (operandIndex == 1)
			return OperandType::VARIABLE_IDX;
	}
	if (op == OpCode::STORE_VAR_R)
	{
		if (operandIndex == 0)
			return OperandType::REGISTER;
		if (operandIndex == 1)
			return OperandType::VARIABLE_IDX;
	}

	// JUMP instructions take an offset (INT)
	if (op == OpCode::JUMP || op == OpCode::JUMP_IF_FALSE || op == OpCode::JUMP_IF_TRUE || op == OpCode::JUMP_BACK)
	{
		return OperandType::INT;
	}

	// Register ops use REGISTER for all operands
	if (static_cast<int>(op) >= static_cast<int>(OpCode::MOV))
	{
		return OperandType::REGISTER;
	}

	return OperandType::INT;
}

std::string PhasorIR::opCodeToString(OpCode op)
{
	auto it = opCodeToStringMap.find(op);
	if (it != opCodeToStringMap.end())
	{
		return it->second;
	}
	return "UNKNOWN";
}

OpCode PhasorIR::stringToOpCode(const std::string &str)
{
	auto it = stringToOpCodeMap.find(str);
	if (it != stringToOpCodeMap.end())
	{
		return it->second;
	}
	throw std::runtime_error("Unknown opcode string: " + str);
}

std::string PhasorIR::escapeString(const std::string &str)
{
	std::stringstream ss;
	for (char c : str)
	{
		switch (c)
		{
		case '\n':
			ss << "\\n";
			break;
		case '\r':
			ss << "\\r";
			break;
		case '\t':
			ss << "\\t";
			break;
		case '\\':
			ss << "\\\\";
			break;
		case '"':
			ss << "\\\"";
			break;
		default:
			ss << c;
			break;
		}
	}
	return ss.str();
}

std::string PhasorIR::unescapeString(const std::string &str)
{
	std::string result;
	for (size_t i = 0; i < str.length(); ++i)
	{
		if (str[i] == '\\' && i + 1 < str.length())
		{
			switch (str[i + 1])
			{
			case 'n':
				result += '\n';
				break;
			case 'r':
				result += '\r';
				break;
			case 't':
				result += '\t';
				break;
			case '\\':
				result += '\\';
				break;
			case '"':
				result += '"';
				break;
			default:
				result += str[i];
				result += str[i + 1];
				break;
			}
			i++;
		}
		else
		{
			result += str[i];
		}
	}
	return result;
}

std::vector<uint8_t> PhasorIR::serialize(const Bytecode &bytecode)
{
	std::stringstream ss;

	// Write Header
	ss << ".PHIR 3.0.0" << "\n";

	// Build reverse lookup maps for inline comments
	std::map<int, std::string> indexToVarName;
	for (const auto &[name, index] : bytecode.variables)
	{
		indexToVarName[index] = name;
	}
	std::map<int, std::string> addressToFuncName;
	for (const auto &[name, address] : bytecode.functionEntries)
	{
		addressToFuncName[address] = name;
	}

	// Constants Section
	ss << ".CONSTANTS " << bytecode.constants.size() << "\n";
	for (const auto &val : bytecode.constants)
	{
		switch (val.getType())
		{
		case ValueType::Null:
			ss << "NULL\n";
			break;
		case ValueType::Bool:
			ss << "BOOL " << (val.asBool() ? "true" : "false") << "\n";
			break;
		case ValueType::Int:
			ss << "INT " << val.asInt() << "\n";
			break;
		case ValueType::Float:
			ss << "FLOAT " << val.asFloat() << "\n";
			break;
		case ValueType::String:
			ss << "STRING \"" << escapeString(val.asString()) << "\"\n";
			break;
		case ValueType::Struct:
			COMPILE_MESSAGE("Warning: PHS_01 Structs have not been fully implemented! Line " STR(__LINE__))
			throw std::runtime_error("Structs not implemented!");
			break;
		case ValueType::Array:
			COMPILE_MESSAGE("Warning: PHS_02 Arrays have not been implemented! Line " STR(__LINE__))
			throw std::runtime_error("Arrays not implemented!");
		}
	}

	// Variables Section
	ss << ".VARIABLES " << bytecode.variables.size() << " " << bytecode.nextVarIndex << "\n";
	for (const auto &[name, index] : bytecode.variables)
	{
		ss << name << " " << index << "\n";
	}

	// Functions Section
	ss << ".FUNCTIONS " << bytecode.functionEntries.size() << "\n";
	for (const auto &[name, address] : bytecode.functionEntries)
	{
		ss << name << " " << address << "\n";
	}

	// Structs Section
	ss << ".STRUCTS " << bytecode.structs.size() << "\n";
	for (const auto &info : bytecode.structs)
	{
		// name firstConstIndex fieldCount fieldName0 fieldName1 ...
		ss << info.name << " " << info.firstConstIndex << " " << info.fieldCount;
		for (const auto &fieldName : info.fieldNames)
		{
			ss << " " << fieldName;
		}
		ss << "\n";
	}

	// Instructions Section
	ss << ".INSTRUCTIONS " << bytecode.instructions.size() << "\n";
	for (const auto &instr : bytecode.instructions)
	{
		std::stringstream instrLine;
		instrLine << opCodeToString(instr.op);

		int     operandCount = getOperandCount(instr.op);
		int32_t operands[5] = {instr.operand1, instr.operand2, instr.operand3, instr.operand4, instr.operand5};

		std::string comment;

		for (int i = 0; i < operandCount; ++i)
		{
			OperandType type = getOperandType(instr.op, i);

			if (i > 0)
				instrLine << ",";
			instrLine << " ";

			switch (type)
			{
			case OperandType::REGISTER:
				instrLine << "r" << operands[i];
				break;
			case OperandType::CONSTANT_IDX:
				instrLine << operands[i];
				if (operands[i] >= 0 && operands[i] < static_cast<int>(bytecode.constants.size()))
				{
					const Value &val = bytecode.constants[operands[i]];
					if (val.getType() == ValueType::String)
					{
						std::string str = val.asString();
						if (str.length() > 20)
							str = str.substr(0, 20) + "...";
						comment = "const[" + std::to_string(operands[i]) + "]=\"" + escapeString(str) + "\"";
					}
					else if (val.getType() == ValueType::Int)
					{
						comment = "const[" + std::to_string(operands[i]) + "]=" + std::to_string(val.asInt());
					}
					else if (val.getType() == ValueType::Float)
					{
						comment = "const[" + std::to_string(operands[i]) + "]=" + std::to_string(val.asFloat());
					}
				}
				break;
			case OperandType::VARIABLE_IDX:
				instrLine << operands[i];
				if (indexToVarName.count(operands[i]))
				{
					comment = "var=" + indexToVarName[operands[i]];
				}
				break;
			case OperandType::FUNCTION_IDX:
				instrLine << operands[i];
				if (addressToFuncName.count(operands[i]))
				{
					comment = "func=" + addressToFuncName[operands[i]];
				}
				break;
			default:
				instrLine << operands[i];
				break;
			}
		}

		std::string lineStr = instrLine.str();
		if (!comment.empty())
		{
			const size_t commentColumn = 40;
			if (lineStr.length() < commentColumn)
			{
				lineStr.append(commentColumn - lineStr.length(), ' ');
			}
			else
			{
				lineStr += " ";
			}

			lineStr += "; " + comment;
		}
		ss << lineStr << "\n";
	}

	std::string          textData = ss.str();
	std::vector<uint8_t> buffer;

	// Append text data
	buffer.insert(buffer.end(), textData.begin(), textData.end());

	return buffer;
}

Bytecode PhasorIR::deserialize(const std::vector<uint8_t> &data)
{
	if (data.size() < 8)
	{
		throw std::runtime_error("Invalid Phasor IR file: too small");
	}

	// Parse text data
	std::string       textData(data.begin() + 8, data.end());
	std::stringstream ss(textData);
	std::string       line;
	Bytecode          bytecode;

	std::string section;
	while (ss >> section)
	{
		if (section == ".PHIR")
		{
			std::string version;
			ss >> version;
			if (version < "1.0.0")
			{
				throw std::runtime_error("Incompatible Phasor IR version");
			}
		}
		if (section == ".CONSTANTS")
		{
			int count;
			ss >> count;
			bytecode.constants.reserve(count);
			for (int i = 0; i < count; ++i)
			{
				std::string type;
				ss >> type;
				if (type == "NULL")
				{
					bytecode.constants.push_back(Value());
				}
				else if (type == "BOOL")
				{
					std::string valStr;
					ss >> valStr;
					bytecode.constants.push_back(Value(valStr == "true"));
				}
				else if (type == "INT")
				{
					int64_t val;
					ss >> val;
					bytecode.constants.push_back(Value(val));
				}
				else if (type == "FLOAT")
				{
					double val;
					ss >> val;
					bytecode.constants.push_back(Value(val));
				}
				else if (type == "STRING")
				{
					std::string valStr;
					// Read quoted string, potentially with spaces
					char c;
					while (ss.get(c) && c != '"')
						;                          // Skip until opening quote
					std::getline(ss, valStr, '"'); // Read until closing quote
					bytecode.constants.push_back(Value(unescapeString(valStr)));
				}
			}
		}
		else if (section == ".VARIABLES")
		{
			int count;
			ss >> count >> bytecode.nextVarIndex;
			for (int i = 0; i < count; ++i)
			{
				std::string name;
				int         index;
				ss >> name >> index;
				bytecode.variables[name] = index;
			}
		}
		else if (section == ".FUNCTIONS")
		{
			int count;
			ss >> count;
			for (int i = 0; i < count; ++i)
			{
				std::string name;
				int         address;
				ss >> name >> address;
				bytecode.functionEntries[name] = address;
			}
		}
		else if (section == ".STRUCTS")
		{
			int count;
			ss >> count;
			for (int i = 0; i < count; ++i)
			{
				StructInfo info;
				ss >> info.name >> info.firstConstIndex >> info.fieldCount;
				info.fieldNames.clear();
				for (int f = 0; f < info.fieldCount; ++f)
				{
					std::string fieldName;
					ss >> fieldName;
					info.fieldNames.push_back(fieldName);
				}
				int index = static_cast<int>(bytecode.structs.size());
				bytecode.structs.push_back(std::move(info));
				bytecode.structEntries[bytecode.structs.back().name] = index;
			}
		}
		else if (section == ".INSTRUCTIONS")
		{
			int count;
			ss >> count;
			bytecode.instructions.reserve(count);
			for (int i = 0; i < count; ++i)
			{
				std::string opStr;
				ss >> opStr;

				OpCode  op = stringToOpCode(opStr);
				int     operandCount = getOperandCount(op);
				int32_t operands[5] = {0, 0, 0, 0, 0};

				for (int j = 0; j < operandCount; ++j)
				{
					std::string token;
					ss >> token;

					// Skip if we hit a comment
					if (token.empty() || token[0] == ';')
					{
						// Skip rest of line
						std::getline(ss, token);
						break;
					}

					// Strip trailing comma if present
					if (!token.empty() && token.back() == ',')
					{
						token.pop_back();
					}

					// Parse register format "rN" or plain integer
					if (!token.empty() && token[0] == 'r')
					{
						operands[j] = std::stoi(token.substr(1));
					}
					else
					{
						operands[j] = std::stoi(token);
					}
				}

				// Skip any remaining content on the line (comments)
				char c;
				while (ss.get(c) && c != '\n')
					;

				bytecode.instructions.push_back(
				    Instruction(op, operands[0], operands[1], operands[2], operands[3], operands[4]));
			}
		}
	}

	return bytecode;
}

bool PhasorIR::saveToFile(const Bytecode &bytecode, const std::filesystem::path &filename)
{
	try
	{
		std::vector<uint8_t> data = serialize(bytecode);
		std::ofstream        file(filename, std::ios::binary);
		if (!file.is_open())
			return false;
		file.write(reinterpret_cast<const char *>(data.data()), data.size());
		return true;
	}
	catch (...)
	{
		return false;
	}
}

Bytecode PhasorIR::loadFromFile(const std::filesystem::path &filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open())
		throw std::runtime_error("Cannot open file");
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<uint8_t> buffer(size);
	if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
		throw std::runtime_error("Cannot read file");
	return deserialize(buffer);
}
} // namespace Phasor
