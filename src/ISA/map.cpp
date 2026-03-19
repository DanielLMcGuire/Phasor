#include "map.hpp"

namespace Phasor {

const std::unordered_map<OpCode, std::string> opCodeToStringMap = {
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
    {OpCode::SYSTEM_ERR_R, "SYSTEM_ERR_R"}
};

const std::unordered_map<std::string, OpCode> stringToOpCodeMap = [] {
	std::unordered_map<std::string, OpCode> map;
	for (const auto &pair : opCodeToStringMap)
	{
		map[pair.second] = pair.first;
	}
	return map;
}();

std::string opCodeToString(OpCode op)
{
	auto it = opCodeToStringMap.find(op);
	if (it != opCodeToStringMap.end())
	{
		return it->second;
	}
	return "UNKNOWN";
}

OpCode stringToOpCode(const std::string &str)
{
	auto it = stringToOpCodeMap.find(str);
	if (it != stringToOpCodeMap.end())
	{
		return it->second;
	}
	throw std::runtime_error("Unknown opcode string: " + str);
}
} // namespace Phasor