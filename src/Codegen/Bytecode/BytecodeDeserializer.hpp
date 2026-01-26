#pragma once
#include "../CodeGen.hpp"
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

namespace Phasor
{

/**
 *  @class BytecodeDeserializer
 *  @brief Bytecode binary format deserializer
 */
class BytecodeDeserializer
{
  public:
	/// @brief Deserialize bytecode from binary buffer
	Bytecode deserialize(const std::vector<uint8_t> &data);

	/// @brief Load bytecode from .phsb file
	Bytecode loadFromFile(const std::filesystem::path &filename);

  private:
	const uint8_t *_data;
	size_t         position;
	size_t         dataSize;

	uint8_t     readUInt8();  ///< Helper method to read UInt8
	uint16_t    readUInt16(); ///< Helper method to read UInt16
	uint32_t    readUInt32(); ///< Helper method to read UInt32
	int32_t     readInt32();  ///< Helper method to read Int32
	int64_t     readInt64();  //< Helper method to read Int64
	double      readDouble(); //< Helper method to read Double
	std::string readString(); //< Helper method to read String

	void readHeader(uint32_t &checksum);          ///< Helper method to read Header
	void readConstantPool(Bytecode &bytecode);    ///< Helper method to read Constants Table
	void readVariableMapping(Bytecode &bytecode); ///< Helper method to read Variable Table
	void readInstructions(Bytecode &bytecode);    ///< Helper method to read Instuctions Table
	void readFunctionEntries(Bytecode &bytecode); ///< Helper method to read Function Entries

	/// @brief Validate header
	void validateHeader(uint32_t expectedChecksum);

	/// @brief Calculate CRC32 checksum
	uint32_t calculateCRC32(const uint8_t *data, size_t size);
};
} // namespace Phasor