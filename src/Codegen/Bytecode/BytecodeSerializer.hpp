#pragma once
#include "../CodeGen.hpp"
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
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
	std::vector<uint8_t> serialize(const Bytecode &bytecode);

	/// @brief Save bytecode to .phsb file
	bool saveToFile(const Bytecode &bytecode, const std::filesystem::path &filename);

  private:
	std::vector<uint8_t> buffer;

	void writeUInt8(uint8_t value);           ///< Helper method to write UInt8
	void writeUInt16(uint16_t value);         ///< Helper method to write UInt16
	void writeUInt32(uint32_t value);         ///< Helper method to write UInt32
	void writeInt32(int32_t value);           ///< Helper method to write Int32
	void writeInt64(int64_t value);           ///< Helper method to write Int64
	void writeDouble(double value);           ///< Helper method to write Double
	void writeString(const std::string &str); ///< Helper method to write String

	/// @brief Section writers
	void writeHeader(uint32_t dataChecksum);                     ///< Helper method to write header
	void writeConstantPool(const std::vector<Value> &constants); ///< Helper method to write Constants Table
	void writeVariableMapping(const std::map<std::string, int> &variables,
	                          int nextVarIndex);                          ///< Helper method to write Variable Map Table
	void writeInstructions(const std::vector<Instruction> &instructions); ///< Helper method to write Instruction Table
	void writeFunctionEntries(
	    const std::map<std::string, int> &functionEntries); ///< Helper method to write Function Table

	/// @brief Calculate CRC32 checksum
	uint32_t calculateCRC32(const std::vector<uint8_t> &data);
};
} // namespace Phasor