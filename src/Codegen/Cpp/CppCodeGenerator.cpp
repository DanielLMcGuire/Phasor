#include "CppCodeGenerator.hpp"
#include "../Bytecode/BytecodeSerializer.hpp"
#include "../Bytecode/BytecodeDeserializer.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>

namespace Phasor
{

bool CppCodeGenerator::generate(const Bytecode &bc, const std::filesystem::path &outputPath, const std::string &modName)
{
	try
	{
		bytecode = &bc;
		output.str("");
		output.clear();

		// Determine module name
		if (modName.empty())
		{
			moduleName = sanitizeModuleName(outputPath.stem().string());
		}
		else
		{
			moduleName = sanitizeModuleName(modName);
		}

		// Serialize bytecode to binary format
		BytecodeSerializer serializer;
		serializedBytecode = serializer.serialize(bc);

		// Generate header file with module name, bytecode, and size
		generateFileHeader();
		generateModuleName();
		generateEmbeddedBytecode();

		// Write to file
		std::ofstream file(outputPath);
		if (!file.is_open())
		{
			return false;
		}

		file << output.str();
		file.close();

		return true;
	}
	catch (const std::exception &)
	{
		return false;
	}
}

Bytecode CppCodeGenerator::generateBytecodeFromEmbedded(const std::string &input)
{
	std::vector<unsigned char> bytecodeData = parseEmbeddedBytecode(input);
	BytecodeDeserializer deserializer;
	return deserializer.deserialize(bytecodeData);
}

void CppCodeGenerator::generateFileHeader()
{
	output << "// Phasor VM Program\n";
	output << "// Module: " << moduleName << "\n";
	output << "#pragma once\n";
	output << "#include <cstddef>\n";
	output << "#include <string>\n\n";
}

void CppCodeGenerator::generateModuleName()
{
	output << "std::string moduleName = \"" << moduleName << "\";\n\n";
}

void CppCodeGenerator::generateEmbeddedBytecode()
{
	output << "inline const unsigned char embeddedBytecode[] = {\n";

	// Write bytecode as hex array
	for (size_t i = 0; i < serializedBytecode.size(); i++)
	{
		if (i % 16 == 0)
			output << "\t";

		output << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(serializedBytecode[i]);

		if (i < serializedBytecode.size() - 1)
			output << ",";

		if (i % 16 == 15)
			output << "\n";
		else if (i < serializedBytecode.size() - 1)
			output << " ";
	}

	output << std::dec << "\n};\n";
	output << "inline const size_t embeddedBytecodeSize = " << serializedBytecode.size() << ";\n";
}

std::vector<unsigned char> CppCodeGenerator::parseEmbeddedBytecode(const std::string &input)
{
    std::vector<unsigned char> result;
    std::istringstream stream(input);
    std::string token;

    while (stream >> token)
    {
        // Only process tokens starting with "0x"
        if (token.size() >= 3 && token[0] == '0' && (token[1] == 'x' || token[1] == 'X'))
        {
            unsigned int byte;
            std::istringstream hexStream(token);
            hexStream >> std::hex >> byte;
            result.push_back(static_cast<unsigned char>(byte));
        }
    }

    return result;
}

std::string CppCodeGenerator::escapeString(const std::string &str)
{
	std::ostringstream escaped;
	for (char c : str)
	{
		switch (c)
		{
		case '\\':
			escaped << "\\\\";
			break;
		case '\"':
			escaped << "\\\"";
			break;
		case '\n':
			escaped << "\\n";
			break;
		case '\r':
			escaped << "\\r";
			break;
		case '\t':
			escaped << "\\t";
			break;
		default:
			if (c >= 32 && c <= 126)
				escaped << c;
			else
				escaped << "\\x" << std::hex << std::setw(2) << std::setfill('0')
				        << static_cast<int>(static_cast<unsigned char>(c));
			break;
		}
	}
	return escaped.str();
}

std::string CppCodeGenerator::getValueTypeString(ValueType type)
{
	switch (type)
	{
	case ValueType::Null:
		return "Null";
	case ValueType::Bool:
		return "Bool";
	case ValueType::Int:
		return "Int";
	case ValueType::Float:
		return "Float";
	case ValueType::String:
		return "String";
	default:
		return "Unknown";
	}
}

std::string CppCodeGenerator::sanitizeModuleName(const std::string &name)
{
	std::string result;
	for (char c : name)
	{
		if (std::isalnum(c) || c == '_')
			result += c;
		else
			result += '_';
	}

	// Ensure it starts with a letter or underscore
	if (!result.empty() && std::isdigit(result[0]))
		result = "_" + result;

	return result.empty() ? "PhasorModule" : result;
}

} // namespace Phasor
