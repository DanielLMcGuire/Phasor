#pragma once
#include "../CodeGen.hpp"
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <phsint.hpp>

/// @brief The Phasor Programming Language and Runtime
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
	Bytecode deserialize(const std::vector<u8> &data);

	/// @brief Load bytecode from .phsb file
	Bytecode loadFromFile(const std::filesystem::path &filename);

  private:
	const u8 *_data;
	size_t    position;
	size_t    dataSize;

	u8          readUInt8();  ///< Helper method to read UInt8
	u16         readUInt16(); ///< Helper method to read UInt16
	u32         readUInt32(); ///< Helper method to read UInt32
	i32         readInt32();  ///< Helper method to read Int32
	i64         readInt64();  ///< Helper method to read Int64
	f64         readDouble(); ///< Helper method to read Double
	std::string readString(); ///< Helper method to read String

	/// @brief Read a single Value (recursive — handles nested structs/arrays)
	Value readValue();

	void readHeader(u32 &checksum);               ///< Helper method to read Header
	void readConstantPool(Bytecode &bytecode);    ///< Helper method to read Constants Table
	void readVariableMapping(Bytecode &bytecode); ///< Helper method to read Variable Table
	void readInstructions(Bytecode &bytecode);    ///< Helper method to read Instructions Table
	void readFunctionEntries(Bytecode &bytecode); ///< Helper method to read Function Entries
	void readFunctionTypes(Bytecode &bytecode);   ///< Helper method to read Function Type Table
	void readStructSection(Bytecode &bytecode);   ///< Helper method to read Struct Section

	/// @brief Calculate CRC32 checksum
	static u32 calculateCRC32(const u8 *data, size_t size);
};
} // namespace Phasor