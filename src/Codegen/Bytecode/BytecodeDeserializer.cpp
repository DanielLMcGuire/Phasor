#include "BytecodeDeserializer.hpp"
#include <cstring>
#include <stdexcept>
#include <filesystem>
#include <phsint.hpp>
#include "metadata.h"

// Section IDs — must match BytecodeSerializer.cpp
const Phasor::u8 SECTION_CONSTANTS    = 0x01;
const Phasor::u8 SECTION_VARIABLES    = 0x02;
const Phasor::u8 SECTION_INSTRUCTIONS = 0x03;
const Phasor::u8 SECTION_FUNCTIONS    = 0x04;
const Phasor::u8 SECTION_STRUCTS      = 0x05;

static Phasor::u32 crc32_table[256];
static bool        crc32_table_initialized = false;

void init_crc32_table_deserializer()
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

u32 BytecodeDeserializer::calculateCRC32(const u8 *data, size_t size)
{
	if (!crc32_table_initialized)
		init_crc32_table_deserializer();

	u32 crc = 0xFFFFFFFF;
	for (size_t i = 0; i < size; i++)
		crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
	return crc ^ 0xFFFFFFFF;
}

// ---------------------------------------------------------------------------
// Primitive readers
// ---------------------------------------------------------------------------

u8 BytecodeDeserializer::readUInt8()
{
	if (position >= dataSize)
		throw std::runtime_error("Unexpected end of bytecode data");
	return _data[position++];
}

u16 BytecodeDeserializer::readUInt16()
{
	u16 value = 0;
	value |= static_cast<u16>(readUInt8());
	value |= static_cast<u16>(readUInt8()) << 8;
	return value;
}

u32 BytecodeDeserializer::readUInt32()
{
	u32 value = 0;
	value |= static_cast<u32>(readUInt8());
	value |= static_cast<u32>(readUInt8()) << 8;
	value |= static_cast<u32>(readUInt8()) << 16;
	value |= static_cast<u32>(readUInt8()) << 24;
	return value;
}

i32 BytecodeDeserializer::readInt32()
{
	return static_cast<i32>(readUInt32());
}

i64 BytecodeDeserializer::readInt64()
{
	i64 value = 0;
	for (int i = 0; i < 8; i++)
		value |= static_cast<i64>(readUInt8()) << (i * 8);
	return value;
}

f64 BytecodeDeserializer::readDouble()
{
	u64 bits = 0;
	for (int i = 0; i < 8; i++)
		bits |= static_cast<u64>(readUInt8()) << (i * 8);
	f64 value;
	std::memcpy(&value, &bits, sizeof(f64));
	return value;
}

std::string BytecodeDeserializer::readString()
{
	u16         length = readUInt16();
	std::string str;
	str.reserve(length);
	for (u16 i = 0; i < length; i++)
		str.push_back(static_cast<char>(readUInt8()));
	return str;
}

// ---------------------------------------------------------------------------
// Recursive value reader — mirrors BytecodeSerializer::writeValue exactly.
// ---------------------------------------------------------------------------
Value BytecodeDeserializer::readValue()
{
	u8 typeTag = readUInt8();

	switch (typeTag)
	{
	case 0: // Null
		return Value{};

	case 1: // Bool
		return Value{readUInt8() != 0};

	case 2: // Int
		return Value{readInt64()};

	case 3: // Float
		return Value{readDouble()};

	case 4: // String
		return Value{readString()};

	case 5: // Struct
	{
		std::string typeName   = readString();
		u32         fieldCount = readUInt32();

		Value structVal = Value::createStruct(PhsString(typeName));
		for (u32 i = 0; i < fieldCount; ++i)
		{
			std::string fieldName = readString();
			Value       fieldVal  = readValue(); // recurse
			structVal.setField(PhsString(fieldName), std::move(fieldVal));
		}
		return structVal;
	}

	case 6: // Array
	{
		u32                elementCount = readUInt32();
		std::vector<Value> elements;
		elements.reserve(elementCount);
		for (u32 i = 0; i < elementCount; ++i)
			elements.push_back(readValue()); // recurse
		return Value::createArray(std::move(elements));
	}

	default:
		throw std::runtime_error("BytecodeDeserializer::readValue: unknown type tag " +
		                         std::to_string(typeTag));
	}
}

// ---------------------------------------------------------------------------
// Section readers
// ---------------------------------------------------------------------------

void BytecodeDeserializer::readHeader(u32 &checksum)
{
	u32 magic = readUInt32();
	if (magic != MAGIC_NUMBER)
		throw std::runtime_error("Invalid bytecode file: incorrect magic number");

	u32 version = readUInt32();
	if (version != VERSION)
		throw std::runtime_error("Incompatible bytecode version");

	u32 flags = readUInt32(); // Reserved
	(void)flags;

	checksum = readUInt32();
}

void BytecodeDeserializer::readConstantPool(Bytecode &bytecode)
{
	u8 sectionId = readUInt8();
	if (sectionId != SECTION_CONSTANTS)
		throw std::runtime_error("Expected constant pool section");

	u32 count = readUInt32();
	bytecode.constants.reserve(count);
	for (u32 i = 0; i < count; i++)
		bytecode.constants.push_back(readValue());
}

void BytecodeDeserializer::readVariableMapping(Bytecode &bytecode)
{
	u8 sectionId = readUInt8();
	if (sectionId != SECTION_VARIABLES)
		throw std::runtime_error("Expected variable mapping section");

	u32 count = readUInt32();
	bytecode.nextVarIndex = readInt32();
	for (u32 i = 0; i < count; i++)
	{
		std::string name  = readString();
		i32         index = readInt32();
		bytecode.variables[name] = index;
	}
}

void BytecodeDeserializer::readInstructions(Bytecode &bytecode)
{
	u8 sectionId = readUInt8();
	if (sectionId != SECTION_INSTRUCTIONS)
		throw std::runtime_error("Expected instructions section");

	u32 count = readUInt32();
	bytecode.instructions.reserve(count);
	for (u32 i = 0; i < count; i++)
	{
		u8  opcode = readUInt8();
		i32 op1    = readInt32();
		i32 op2    = readInt32();
		i32 op3    = readInt32();
		bytecode.instructions.emplace_back(static_cast<OpCode>(opcode), op1, op2, op3);
	}
}

void BytecodeDeserializer::readFunctionEntries(Bytecode &bytecode)
{
	u8 sectionId = readUInt8();
	if (sectionId != SECTION_FUNCTIONS)
		throw std::runtime_error("Expected function entries section");

	u32 count = readUInt32();
	for (u32 i = 0; i < count; i++)
	{
		std::string name    = readString();
		i32         address = readInt32();
		bytecode.functionEntries[name] = address;
	}
}

/**
 * @brief Read struct type definitions section (0x05).
 *
 * Reconstructs bytecode.structs and bytecode.structEntries.
 */
void BytecodeDeserializer::readStructSection(Bytecode &bytecode)
{
	u8 sectionId = readUInt8();
	if (sectionId != SECTION_STRUCTS)
		throw std::runtime_error("Expected struct definitions section");

	u32 structCount = readUInt32();
	bytecode.structs.reserve(structCount);

	for (u32 i = 0; i < structCount; i++)
	{
		StructInfo info;
		info.name            = readString();
		info.firstConstIndex = readInt32();
		info.fieldCount      = readInt32();

		info.fieldNames.reserve(static_cast<size_t>(info.fieldCount));
		for (int f = 0; f < info.fieldCount; f++)
			info.fieldNames.push_back(readString());

		int index = static_cast<int>(bytecode.structs.size());
		bytecode.structs.push_back(std::move(info));
		bytecode.structEntries[bytecode.structs.back().name] = index;
	}
}

// ---------------------------------------------------------------------------
// Top-level deserialize / loadFromFile
// ---------------------------------------------------------------------------

Bytecode BytecodeDeserializer::deserialize(const std::vector<u8> &buffer)
{
	_data    = buffer.data();
	dataSize = buffer.size();
	position = 0;

	Bytecode bytecode;

	u32 expectedChecksum;
	readHeader(expectedChecksum);

	size_t dataStart      = position;
	u32    actualChecksum = calculateCRC32(_data + dataStart, dataSize - dataStart);
	if (actualChecksum != expectedChecksum)
		throw std::runtime_error("Bytecode file corrupted: checksum mismatch");

	// Sections must be read in the same order they were written.
	readConstantPool(bytecode);
	readVariableMapping(bytecode);
	readFunctionEntries(bytecode);
	readStructSection(bytecode);
	readInstructions(bytecode);

	return bytecode;
}

Bytecode BytecodeDeserializer::loadFromFile(const std::filesystem::path &filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open())
		throw std::runtime_error("Failed to open bytecode file: " + filename.string());

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<u8> fileBuffer(size);
	if (!file.read(reinterpret_cast<char *>(fileBuffer.data()), size))
		throw std::runtime_error("Failed to read bytecode file: " + filename.string());

	return deserialize(fileBuffer);
}

} // namespace Phasor