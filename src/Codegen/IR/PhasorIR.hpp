#pragma once
#include "../CodeGen.hpp"
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @class PhasorIR
 * @brief Phasor IR Serializer/Deserializer
 */
class PhasorIR
{
  public:
	/// @brief Serialize bytecode to Phasor IR format
	static std::vector<uint8_t> serialize(const Bytecode &bytecode);

	/// @brief Deserialize Phasor IR format to bytecode
	static Bytecode deserialize(const std::vector<uint8_t> &data);

	/// @brief Save bytecode to .phir file
	static bool saveToFile(const Bytecode &bytecode, const std::filesystem::path &filename);

	/// @brief Load bytecode from .phir file
	static Bytecode loadFromFile(const std::filesystem::path &filename);

	/// @brief Helper to convert OpCode to string
	static std::string opCodeToString(OpCode op);

	/// @brief Helper to convert string to OpCode
	static OpCode stringToOpCode(const std::string &str);

	/// @brief Helper to escape strings for text format
	static std::string escapeString(const std::string &str);

	/// @brief Helper to unescape strings from text format
	static std::string unescapeString(const std::string &str);

  private:
	enum class OperandType
	{
		NONE,
		INT,
		REGISTER,
		CONSTANT_IDX,
		VARIABLE_IDX,
		FUNCTION_IDX
	};

	static int         getOperandCount(OpCode op);
	static OperandType getOperandType(OpCode op, int operandIndex);

	static const std::unordered_map<OpCode, std::string> opCodeToStringMap;
	static const std::unordered_map<std::string, OpCode> stringToOpCodeMap;
};
