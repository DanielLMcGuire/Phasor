#pragma once
#include "../CodeGen.hpp"
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <phsint.hpp>

/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/**
 * @class BytecodeSerializer
 * @brief Bytecode binary format serializer
 */
class BytecodeSerializer
{
  public:
	/// @brief Serialize bytecode to binary buffer
	std::vector<u8> serialize(const Bytecode &bytecode);

	/// @brief Save bytecode to .phsb file
	bool saveToFile(const Bytecode &bytecode, const std::filesystem::path &filename);

  private:
	std::vector<u8> buffer;

	void writeUInt8(u8 value);           ///< Helper method to write UInt8
	void writeUInt16(u16 value);         ///< Helper method to write UInt16
	void writeUInt32(u32 value);         ///< Helper method to write UInt32
	void writeInt32(i32 value);           ///< Helper method to write Int32
	void writeInt64(i64 value);           ///< Helper method to write Int64
	void writeDouble(f64 value);           ///< Helper method to write Double
	void writeString(const std::string &str); ///< Helper method to write String

	/// @brief Section writers
	void writeHeader(u32 dataChecksum);                     ///< Helper method to write header
	void writeConstantPool(const std::vector<Value> &constants); ///< Helper method to write Constants Table
	void writeVariableMapping(const std::unordered_map<std::string, int> &variables,
	                          int nextVarIndex);                          ///< Helper method to write Variable Map Table
	void writeInstructions(const std::vector<Instruction> &instructions); ///< Helper method to write Instruction Table
	void writeFunctionEntries(
	    const std::unordered_map<std::string, int> &functionEntries); ///< Helper method to write Function Table

	/// @brief Calculate CRC32 checksum
	static u32 calculateCRC32(const std::vector<u8> &data);
};
} // namespace Phasor