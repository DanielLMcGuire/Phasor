#pragma once
#include "../CodeGen.hpp"
#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>
#include <phsint.hpp>

/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/**
 * @class PhasorIR
 * @brief Phasor IR Serializer/Deserializer
 */
class PhasorIR
{
  public:
	/// @brief Serialize bytecode to Phasor IR format
	static std::vector<u8> serialize(const Bytecode &bytecode);

	/// @brief Deserialize Phasor IR format to bytecode
	static Bytecode deserialize(const std::vector<u8> &data);

	/// @brief Save bytecode to .phir file
	static bool saveToFile(const Bytecode &bytecode, const std::filesystem::path &filename);

	/// @brief Load bytecode from .phir file
	static Bytecode loadFromFile(const std::filesystem::path &filename);

	/// @brief Helper to escape strings for text format
	static std::string escapeString(const std::string &str);

	/// @brief Helper to unescape strings from text format
	static std::string unescapeString(const std::string &str);

  private:
	/// @brief Operand types for instructions
	enum class OperandType : u8
	{
		NONE,         ///< No operand
		INT,          ///< Integer operand
		REGISTER,     ///< Register operand
		CONSTANT_IDX, ///< Index into constant pool
		VARIABLE_IDX, ///< Index into variable mapping
		FUNCTION_IDX  ///< Index into function entries
	};

	static int         getOperandCount(OpCode op);
	static OperandType getOperandType(OpCode op, int operandIndex);

	static const std::unordered_map<OpCode, std::string> opCodeToStringMap;
	static const std::unordered_map<std::string, OpCode> stringToOpCodeMap;
};

} // namespace Phasor