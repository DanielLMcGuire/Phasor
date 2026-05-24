#include "BytecodeSerializer.hpp"
#include <cstring>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include "metadata.h"
#include <phsint.hpp>

// Section IDs
const Phasor::u8 SECTION_CONSTANTS    = 0x01;
const Phasor::u8 SECTION_VARIABLES    = 0x02;
const Phasor::u8 SECTION_INSTRUCTIONS = 0x03;
const Phasor::u8 SECTION_FUNCTIONS    = 0x04;
const Phasor::u8 SECTION_STRUCTS      = 0x05;

static Phasor::u32 crc32_table[256];
static bool        crc32_table_initialized = false;

void init_crc32_table()
{
	for (Phasor::u32 i = 0; i < 256; i++)
	{
		Phasor::u32 crc = i;
		for (int j = 0; j < 8; j++)
		{
			if ((crc & 1) != 0u)
				crc = (crc >> 1) ^ 0xEDB88320;
			else
				crc >>= 1;
		}
		crc32_table[i] = crc;
	}
	crc32_table_initialized = true;
}

namespace Phasor
{

u32 BytecodeSerializer::calculateCRC32(const std::vector<u8> &data)
{
	if (!crc32_table_initialized)
		init_crc32_table();

	u32 crc = 0xFFFFFFFF;
	for (u8 byte : data)
		crc = (crc >> 8) ^ crc32_table[(crc ^ byte) & 0xFF];
	return crc ^ 0xFFFFFFFF;
}

// ---------------------------------------------------------------------------
// Primitive writers
// ---------------------------------------------------------------------------

void BytecodeSerializer::writeUInt8(u8 value)
{
	buffer.push_back(value);
}

void BytecodeSerializer::writeUInt16(u16 value)
{
	buffer.push_back(static_cast<u8>(value & 0xFF));
	buffer.push_back(static_cast<u8>((value >> 8) & 0xFF));
}

void BytecodeSerializer::writeUInt32(u32 value)
{
	buffer.push_back(static_cast<u8>(value & 0xFF));
	buffer.push_back(static_cast<u8>((value >> 8) & 0xFF));
	buffer.push_back(static_cast<u8>((value >> 16) & 0xFF));
	buffer.push_back(static_cast<u8>((value >> 24) & 0xFF));
}

void BytecodeSerializer::writeInt32(i32 value)
{
	writeUInt32(static_cast<u32>(value));
}

void BytecodeSerializer::writeInt64(i64 value)
{
	for (int i = 0; i < 8; i++)
		buffer.push_back(static_cast<u8>((value >> (i * 8)) & 0xFF));
}

void BytecodeSerializer::writeDouble(f64 value)
{
	u64 bits;
	std::memcpy(&bits, &value, sizeof(f64));
	for (int i = 0; i < 8; i++)
		buffer.push_back(static_cast<u8>((bits >> (i * 8)) & 0xFF));
}

void BytecodeSerializer::writeString(const std::string &str)
{
	writeUInt16(static_cast<u16>(str.length()));
	for (char c : str)
		buffer.push_back(static_cast<u8>(c));
}

// ---------------------------------------------------------------------------
// Recursive value writer
//
// Binary layout per value:
//   Null   : u8(0)
//   Bool   : u8(1)  u8(0|1)
//   Int    : u8(2)  i64
//   Float  : u8(3)  f64
//   String : u8(4)  u16(len) chars...
//   Struct : u8(5)  string(structName) u32(fieldCount) [string(fieldName) value]...
//   Array  : u8(6)  u32(elementCount) [value]...
// ---------------------------------------------------------------------------
void BytecodeSerializer::writeValue(const Value &val)
{
	switch (val.getType())
	{
	case ValueType::Null:
		writeUInt8(0);
		break;

	case ValueType::Bool:
		writeUInt8(1);
		writeUInt8(val.asBool() ? 1 : 0);
		break;

	case ValueType::Int:
		writeUInt8(2);
		writeInt64(val.asInt());
		break;

	case ValueType::Float:
		writeUInt8(3);
		writeDouble(val.asFloat());
		break;

	case ValueType::String:
		writeUInt8(4);
		writeString(val.asString().str());
		break;

	case ValueType::Struct:
	{
		auto s = val.asStruct();
		writeUInt8(5);
		writeString(s->structName.str());
		writeUInt32(static_cast<u32>(s->fields.size()));
		for (const auto &[fieldName, fieldVal] : s->fields)
		{
			writeString(fieldName.str());
			writeValue(fieldVal); // recurse
		}
		break;
	}

	case ValueType::Array:
	{
		auto a = val.asArray();
		writeUInt8(6);
		writeUInt32(static_cast<u32>(a->size()));
		for (const auto &elem : *a)
			writeValue(elem); // recurse
		break;
	}

	default:
		throw std::runtime_error("BytecodeSerializer::writeValue: unknown ValueType");
	}
}

// ---------------------------------------------------------------------------
// Section writers
// ---------------------------------------------------------------------------

void BytecodeSerializer::writeHeader(u32 dataChecksum)
{
	writeUInt32(MAGIC_NUMBER);
	writeUInt32(VERSION);
	writeUInt32(0); // Flags (reserved)
	writeUInt32(dataChecksum);
}

void BytecodeSerializer::writeConstantPool(const std::vector<Value> &constants)
{
	writeUInt8(SECTION_CONSTANTS);
	writeUInt32(static_cast<u32>(constants.size()));
	for (const auto &val : constants)
		writeValue(val);
}

void BytecodeSerializer::writeVariableMapping(const std::unordered_map<std::string, int> &variables,
                                              int nextVarIndex)
{
	writeUInt8(SECTION_VARIABLES);
	writeUInt32(static_cast<u32>(variables.size()));
	writeInt32(nextVarIndex);
	for (const auto &[name, index] : variables)
	{
		writeString(name);
		writeInt32(index);
	}
}

void BytecodeSerializer::writeInstructions(const std::vector<Instruction> &instructions)
{
	writeUInt8(SECTION_INSTRUCTIONS);
	writeUInt32(static_cast<u32>(instructions.size()));
	for (const auto &instr : instructions)
	{
		writeUInt8(static_cast<u8>(instr.op));
		writeInt32(instr.operand1);
		writeInt32(instr.operand2);
		writeInt32(instr.operand3);
	}
}

void BytecodeSerializer::writeFunctionEntries(const std::unordered_map<std::string, int> &functionEntries)
{
	writeUInt8(SECTION_FUNCTIONS);
	writeUInt32(static_cast<u32>(functionEntries.size()));
	for (const auto &[name, address] : functionEntries)
	{
		writeString(name);
		writeInt32(address);
	}
}

/**
 * @brief Write struct type definitions section (0x05).
 *
 * Binary layout:
 *   u8     section_id = 0x05
 *   u32    structCount
 *   for each StructInfo:
 *     string  name
 *     i32     firstConstIndex
 *     i32     fieldCount
 *     string  fieldName[0..fieldCount-1]
 */
void BytecodeSerializer::writeStructSection(const std::vector<StructInfo> &structs)
{
	writeUInt8(SECTION_STRUCTS);
	writeUInt32(static_cast<u32>(structs.size()));
	for (const auto &info : structs)
	{
		writeString(info.name);
		writeInt32(info.firstConstIndex);
		writeInt32(info.fieldCount);
		for (const auto &fieldName : info.fieldNames)
			writeString(fieldName);
	}
}

// ---------------------------------------------------------------------------
// Top-level serialize / saveToFile
// ---------------------------------------------------------------------------

std::vector<u8> BytecodeSerializer::serialize(const Bytecode &bytecode)
{
	buffer.clear();

	// Reserve 16 bytes for the header (written after checksum is known).
	for (int i = 0; i < 16; i++)
		buffer.push_back(0);

	size_t dataStartPos = buffer.size();
	writeConstantPool(bytecode.constants);
	writeVariableMapping(bytecode.variables, bytecode.nextVarIndex);
	writeFunctionEntries(bytecode.functionEntries);
	writeStructSection(bytecode.structs);
	writeInstructions(bytecode.instructions);

	std::vector<u8> dataSection(buffer.begin() + dataStartPos, buffer.end());
	u32             checksum = calculateCRC32(dataSection);

	std::vector<u8> tempBuffer = buffer;
	buffer.clear();
	writeHeader(checksum);
	buffer.insert(buffer.end(), tempBuffer.begin() + 16, tempBuffer.end());

	return buffer;
}

bool BytecodeSerializer::saveToFile(const Bytecode &bytecode, const std::filesystem::path &filename)
{
	try
	{
		std::vector<u8> data = serialize(bytecode);
		std::ofstream   file(filename, std::ios::binary);
		if (!file.is_open())
			return false;
		file.write(reinterpret_cast<const char *>(data.data()), data.size());
		return true;
	}
	catch (const std::exception &)
	{
		return false;
	}
}

} // namespace Phasor