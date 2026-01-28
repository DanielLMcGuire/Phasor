#include "BytecodeSerializer.hpp"
#include <cstring>
#include <stdexcept>
#include <filesystem>
#include <fstream>

/**
 * @brief Magic number (little endian)
 *
 * 'PHSB'
 */
const uint32_t MAGIC_NUMBER = 0x42534850;

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

/// @brief Init CRC32 Table
void init_crc32_table()
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

namespace Phasor
{

uint32_t BytecodeSerializer::calculateCRC32(const std::vector<uint8_t> &data)
{
	if (!crc32_table_initialized)
	{
		init_crc32_table();
	}

	uint32_t crc = 0xFFFFFFFF;
	for (uint8_t byte : data)
	{
		crc = (crc >> 8) ^ crc32_table[(crc ^ byte) & 0xFF];
	}
	return crc ^ 0xFFFFFFFF;
}

void BytecodeSerializer::writeUInt8(uint8_t value)
{
	buffer.push_back(value);
}

void BytecodeSerializer::writeUInt16(uint16_t value)
{
	buffer.push_back(static_cast<uint8_t>(value & 0xFF));
	buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
}

void BytecodeSerializer::writeUInt32(uint32_t value)
{
	buffer.push_back(static_cast<uint8_t>(value & 0xFF));
	buffer.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
	buffer.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
	buffer.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
}

void BytecodeSerializer::writeInt32(int32_t value)
{
	writeUInt32(static_cast<uint32_t>(value));
}

void BytecodeSerializer::writeInt64(int64_t value)
{
	for (int i = 0; i < 8; i++)
	{
		buffer.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
	}
}

void BytecodeSerializer::writeDouble(double value)
{
	uint64_t bits;
	std::memcpy(&bits, &value, sizeof(double));
	for (int i = 0; i < 8; i++)
	{
		buffer.push_back(static_cast<uint8_t>((bits >> (i * 8)) & 0xFF));
	}
}

void BytecodeSerializer::writeString(const std::string &str)
{
	writeUInt16(static_cast<uint16_t>(str.length()));
	for (char c : str)
	{
		buffer.push_back(static_cast<uint8_t>(c));
	}
}

void BytecodeSerializer::writeHeader(uint32_t dataChecksum)
{
	writeUInt32(MAGIC_NUMBER);
	writeUInt32(VERSION);
	writeUInt32(0); // Flags (reserved)
	writeUInt32(dataChecksum);
}

void BytecodeSerializer::writeConstantPool(const std::vector<Value> &constants)
{
	writeUInt8(SECTION_CONSTANTS);
	writeUInt32(static_cast<uint32_t>(constants.size()));

	for (const auto &constant : constants)
	{
		ValueType type = constant.getType();

		// Write type tag
		switch (type)
		{
		case ValueType::Null:
			writeUInt8(0);
			break;
		case ValueType::Bool:
			writeUInt8(1);
			writeUInt8(constant.asBool() ? 1 : 0);
			break;
		case ValueType::Int:
			writeUInt8(2);
			writeInt64(constant.asInt());
			break;
		case ValueType::Float:
			writeUInt8(3);
			writeDouble(constant.asFloat());
			break;
		case ValueType::String:
			writeUInt8(4);
			writeString(constant.asString());
			break;
		case ValueType::Struct:
#ifdef _MSC_VER
#pragma message("Warning: PHS_01 Structs have not been implemented!")
#else 
#warning "PHS_01 Structs have not been implemented!"
#endif
			throw std::runtime_error("Structs have not been implemented!");
			break;
		case ValueType::Array:
#ifdef _MSC_VER
#pragma message("Warning: PHS_02 Arrays have not been implemented!")
#else 
#warning "PHS_02 Arrays have not been implemented!"
#endif
			throw std::runtime_error("Arrays have not been implemented!");
			break;
		}

	}
}

void BytecodeSerializer::writeVariableMapping(const std::map<std::string, int> &variables, int nextVarIndex)
{
	writeUInt8(SECTION_VARIABLES);
	writeUInt32(static_cast<uint32_t>(variables.size()));
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
	writeUInt32(static_cast<uint32_t>(instructions.size()));

	for (const auto &instr : instructions)
	{
		writeUInt8(static_cast<uint8_t>(instr.op));
		writeInt32(instr.operand1);
		writeInt32(instr.operand2);
		writeInt32(instr.operand3);
		writeInt32(instr.operand4);
		writeInt32(instr.operand5);
	}
}

void BytecodeSerializer::writeFunctionEntries(const std::map<std::string, int> &functionEntries)
{
	writeUInt8(SECTION_FUNCTIONS);
	writeUInt32(static_cast<uint32_t>(functionEntries.size()));

	for (const auto &[name, address] : functionEntries)
	{
		writeString(name);
		writeInt32(address);
	}
}

std::vector<uint8_t> BytecodeSerializer::serialize(const Bytecode &bytecode)
{
	buffer.clear();

	for (int i = 0; i < 16; i++)
	{
		buffer.push_back(0);
	}

	// Write all sections
	size_t dataStartPos = buffer.size();
	writeConstantPool(bytecode.constants);
	writeVariableMapping(bytecode.variables, bytecode.nextVarIndex);
	writeFunctionEntries(bytecode.functionEntries);
	writeInstructions(bytecode.instructions);

	// Calculate checksum of data (everything after header)
	std::vector<uint8_t> dataSection(buffer.begin() + dataStartPos, buffer.end());
	uint32_t             checksum = calculateCRC32(dataSection);

	// Write header at the beginning
	std::vector<uint8_t> tempBuffer = buffer;
	buffer.clear();
	writeHeader(checksum);

	// Append the data sections
	buffer.insert(buffer.end(), tempBuffer.begin() + 16, tempBuffer.end());

	return buffer;
}

bool BytecodeSerializer::saveToFile(const Bytecode &bytecode, const std::filesystem::path &filename)
{
	try
	{
		std::vector<uint8_t> data = serialize(bytecode);

		std::ofstream file(filename, std::ios::binary);
		if (!file.is_open())
		{
			return false;
		}

		file.write(reinterpret_cast<const char *>(data.data()), data.size());
		file.close();

		return true;
	}
	catch (const std::exception &)
	{
		return false;
	}
}
} // namespace Phasor
