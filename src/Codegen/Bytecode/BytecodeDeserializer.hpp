// Copyright 2025-2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://phasor.pages.dev/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include "../Bytecode.hpp"
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
	void readScopeVars(Bytecode &bytecode);       ///< Helper method to read Scope Vars

	/// @brief Calculate CRC32 checksum
	static u32 calculateCRC32(const u8 *data, size_t size);
};
} // namespace Phasor