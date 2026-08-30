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
	static Phasor::string escapeString(const Phasor::string &str);

	/// @brief Helper to unescape strings from text format
	static Phasor::string unescapeString(const Phasor::string &str);

  private:
	/// @brief Operand types for instructions
	enum class OperandType : u8
	{
		NONE,         ///< No operand
		INT,          ///< Integer operand
		REGISTER,     ///< Register operand
		CONSTANT_IDX, ///< Index into constant pool
		VARIABLE_IDX, ///< Index into variable mapping
		FUNCTION_IDX,  ///< Index into function entries
		SCOPE_IDX
	};

	static int         getOperandCount(OpCode op);
	static OperandType getOperandType(OpCode op, int operandIndex);

	static const std::unordered_map<OpCode, Phasor::string> opCodeToStringMap;
	static const std::unordered_map<Phasor::string, OpCode> stringToOpCodeMap;
};

} // namespace Phasor