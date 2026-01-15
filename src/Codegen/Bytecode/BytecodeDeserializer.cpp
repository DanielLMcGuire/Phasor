#include "BytecodeDeserializer.hpp"
#include <cstring>
#include <stdexcept>
#include <filesystem>

/**
 * @brief Magic number
 *
 * 'PHSB'
 */
const uint32_t MAGIC_NUMBER = 0x50485342;

/**
 * @brief Version number
 *
 * '3.0.0.0'
 */
const uint32_t VERSION = 0x03000000;

// Section IDs
const uint8_t SECTION_CONSTANTS = 0x01;    //< Constants Section
const uint8_t SECTION_VARIABLES = 0x02;    //< Variables Section
const uint8_t SECTION_INSTRUCTIONS = 0x03; //< Instructions Section
const uint8_t SECTION_FUNCTIONS = 0x04;    //< Functions Section

static uint32_t crc32_table[256]; //< CRC32 lookup table
static bool     crc32_table_initialized = false;

/// @brief Deserialize CRC32 Table
void init_crc32_table_deserializer()
{
	for (uint32_t i = 0; i < 256; i++)
	{
		uint32_t crc = i;
		for (int j = 0; j < 8; j++)
		{
			if (crc & 1)
			{
				crc = (crc >> 1) ^ 0xEDB88320;
			}
			else
			{
				crc >>= 1;
			}
		}
		crc32_table[i] = crc;
	}
	crc32_table_initialized = true;
}
uint32_t BytecodeDeserializer::calculateCRC32(const uint8_t *data, size_t size)
{
	if (!crc32_table_initialized)
	{
		init_crc32_table_deserializer();
	}

	uint32_t crc = 0xFFFFFFFF;
	for (size_t i = 0; i < size; i++)
	{
		crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xFF];
	}
	return crc ^ 0xFFFFFFFF;
}

uint8_t BytecodeDeserializer::readUInt8()
{
	if (position >= dataSize)
	{
		throw std::runtime_error("Unexpected end of bytecode data");
	}
	return data[position++];
}

uint16_t BytecodeDeserializer::readUInt16()
{
	uint16_t value = 0;
	value |= static_cast<uint16_t>(readUInt8());
	value |= static_cast<uint16_t>(readUInt8()) << 8;
	return value;
}

uint32_t BytecodeDeserializer::readUInt32()
{
	uint32_t value = 0;
	value |= static_cast<uint32_t>(readUInt8());
	value |= static_cast<uint32_t>(readUInt8()) << 8;
	value |= static_cast<uint32_t>(readUInt8()) << 16;
	value |= static_cast<uint32_t>(readUInt8()) << 24;
	return value;
}

int32_t BytecodeDeserializer::readInt32()
{
	return static_cast<int32_t>(readUInt32());
}

int64_t BytecodeDeserializer::readInt64()
{
	int64_t value = 0;
	for (int i = 0; i < 8; i++)
	{
		value |= static_cast<int64_t>(readUInt8()) << (i * 8);
	}
	return value;
}

double BytecodeDeserializer::readDouble()
{
	uint64_t bits = 0;
	for (int i = 0; i < 8; i++)
	{
		bits |= static_cast<uint64_t>(readUInt8()) << (i * 8);
	}
	double value;
	std::memcpy(&value, &bits, sizeof(double));
	return value;
}

std::string BytecodeDeserializer::readString()
{
	uint16_t    length = readUInt16();
	std::string str;
	str.reserve(length);
	for (uint16_t i = 0; i < length; i++)
	{
		str.push_back(static_cast<char>(readUInt8()));
	}
	return str;
}

void BytecodeDeserializer::readHeader(uint32_t &checksum)
{
	uint32_t magic = readUInt32();
	if (magic != MAGIC_NUMBER)
	{
		throw std::runtime_error("Invalid bytecode file: incorrect magic number");
	}

	uint32_t version = readUInt32();
	if (version != VERSION)
	{
		throw std::runtime_error("Incompatible bytecode version");
	}

	uint32_t flags = readUInt32(); // Reserved for future use
	(void)flags;

	checksum = readUInt32();
}

void BytecodeDeserializer::readConstantPool(Bytecode &bytecode)
{
	uint8_t sectionId = readUInt8();
	if (sectionId != SECTION_CONSTANTS)
	{
		throw std::runtime_error("Expected constant pool section");
	}

	uint32_t count = readUInt32();
	bytecode.constants.reserve(count);

	for (uint32_t i = 0; i < count; i++)
	{
		uint8_t typeTag = readUInt8();

		switch (typeTag)
		{
		case 0: // Null
			bytecode.constants.push_back(Value());
			break;
		case 1: // Bool
		{
			uint8_t boolValue = readUInt8();
			bytecode.constants.push_back(Value(boolValue != 0));
			break;
		}
		case 2: // Int
		{
			int64_t intValue = readInt64();
			bytecode.constants.push_back(Value(intValue));
			break;
		}
		case 3: // Float
		{
			double floatValue = readDouble();
			bytecode.constants.push_back(Value(floatValue));
			break;
		}
		case 4: // String
		{
			std::string strValue = readString();
			bytecode.constants.push_back(Value(strValue));
			break;
		}
		default:
			throw std::runtime_error("Unknown value type in constant pool");
		}
	}
}

void BytecodeDeserializer::readVariableMapping(Bytecode &bytecode)
{
	uint8_t sectionId = readUInt8();
	if (sectionId != SECTION_VARIABLES)
	{
		throw std::runtime_error("Expected variable mapping section");
	}

	uint32_t count = readUInt32();
	bytecode.nextVarIndex = readInt32();

	for (uint32_t i = 0; i < count; i++)
	{
		std::string name = readString();
		int32_t     index = readInt32();
		bytecode.variables[name] = index;
	}
}

void BytecodeDeserializer::readInstructions(Bytecode &bytecode)
{
	uint8_t sectionId = readUInt8();
	if (sectionId != SECTION_INSTRUCTIONS)
	{
		throw std::runtime_error("Expected instructions section");
	}

	uint32_t count = readUInt32();
	bytecode.instructions.reserve(count);

	for (uint32_t i = 0; i < count; i++)
	{
		uint8_t opcode = readUInt8();
		int32_t op1 = readInt32();
		int32_t op2 = readInt32();
		int32_t op3 = readInt32();
		int32_t op4 = readInt32();
		int32_t op5 = readInt32();
		bytecode.instructions.push_back(Instruction(static_cast<OpCode>(opcode), op1, op2, op3, op4, op5));
	}
}

void BytecodeDeserializer::readFunctionEntries(Bytecode &bytecode)
{
	uint8_t sectionId = readUInt8();
	if (sectionId != SECTION_FUNCTIONS)
	{
		throw std::runtime_error("Expected function entries section");
	}

	uint32_t count = readUInt32();

	for (uint32_t i = 0; i < count; i++)
	{
		std::string name = readString();
		int32_t     address = readInt32();
		bytecode.functionEntries[name] = address;
	}
}

Bytecode BytecodeDeserializer::deserialize(const std::vector<uint8_t> &buffer)
{
	data = buffer.data();
	dataSize = buffer.size();
	position = 0;

	Bytecode bytecode;

	// Read header
	uint32_t expectedChecksum;
	readHeader(expectedChecksum);

	// Calculate checksum of data section
	size_t   dataStart = position;
	uint32_t actualChecksum = calculateCRC32(data + dataStart, dataSize - dataStart);

	if (actualChecksum != expectedChecksum)
	{
		throw std::runtime_error("Bytecode file corrupted: checksum mismatch");
	}

	// Read sections
	readConstantPool(bytecode);
	readVariableMapping(bytecode);
	readFunctionEntries(bytecode);
	readInstructions(bytecode);

	return bytecode;
}

Bytecode BytecodeDeserializer::loadFromFile(const std::filesystem::path &filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open bytecode file: " + filename.string());
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<uint8_t> buffer(size);
	if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
	{
		throw std::runtime_error("Failed to read bytecode file: " + filename.string());
	}

	file.close();

	return deserialize(buffer);
}
