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

#include "CCodeGenerator.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>

namespace Phasor
{

bool CCodeGenerator::generate(const unsigned char *bytecodeData, size_t bytecodeSize,
                               const std::filesystem::path &outputPath, const Phasor::string &modName)
{
	try
	{
		output.str("");
		output.clear();

		// Determine module name
		if (modName.empty())
		{
			moduleName = sanitizeModuleName(outputPath.stem().string());
		} else {
			moduleName = sanitizeModuleName(modName);
		}

		serializedBytecode.assign(bytecodeData, bytecodeData + bytecodeSize);

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

std::vector<unsigned char> CCodeGenerator::generateBytecodeFromEmbedded(const Phasor::string &input)
{
	return parseEmbeddedBytecode(input);
}

void CCodeGenerator::generateFileHeader()
{
	output << "// Phasor VM Program\n";
	output << "// Module: " << moduleName << "\n";
	output << "#pragma once\n";
	output << "#include <stddef.h>\n";
}

void CCodeGenerator::generateModuleName()
{
	output << "const char *moduleName = \"" << moduleName << "\";\n\n";
}

void CCodeGenerator::generateEmbeddedBytecode()
{
#if defined(_WIN32)
	const Phasor::string sectionPrefixPragma = "#pragma section(\".phsb\", read)\n";
	const Phasor::string sectionAttr = "__declspec(allocate(\".phsb\")) ";
#elif defined(__APPLE__)
	const Phasor::string sectionPrefixPragma = "";
	const Phasor::string sectionAttr = "__attribute__((section(\"__DATA,__phsb\"))) ";
#elif defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	const Phasor::string sectionPrefixPragma = "";
	const Phasor::string sectionAttr = "__attribute__((section(\".phsb\"))) ";
#else
	const Phasor::string sectionPrefixPragma = "";
	const Phasor::string sectionAttr = "";
#endif

	if (!sectionPrefixPragma.empty())
	{
		output << sectionPrefixPragma;
	}

	output << sectionAttr << "const size_t embeddedBytecodeSize = " << serializedBytecode.size() << ";\n";

	output << sectionAttr << "const unsigned char embeddedBytecode[] = {\n";

	for (size_t i = 0; i < serializedBytecode.size(); i++)
	{
		if (i % 16 == 0)
		{
			output << "\t";
		}

		output << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(serializedBytecode[i]);

		if (i < serializedBytecode.size() - 1)
		{
			output << ",";
		}

		if (i % 16 == 15)
		{
			output << "\n";
		}
		else if (i < serializedBytecode.size() - 1)
		{
			output << " ";
		}
	}

	output << std::dec << "\n};\n";
}

std::vector<unsigned char> CCodeGenerator::parseEmbeddedBytecode(const Phasor::string &input)
{
	std::vector<unsigned char> result;
	std::istringstream         stream(input);
	std::string                token;

	while (stream >> token)
	{
		if (token.size() >= 3 && token[0] == '0' && (token[1] == 'x' || token[1] == 'X'))
		{
			unsigned int       byte;
			std::istringstream hexStream(token);
			hexStream >> std::hex >> byte;
			result.push_back(static_cast<unsigned char>(byte));
		}
	}

	return result;
}

Phasor::string CCodeGenerator::escapeString(const Phasor::string &str)
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
			{
				escaped << c;
			} else {
				escaped << "\\x" << std::hex << std::setw(2) << std::setfill('0')
				        << static_cast<int>(static_cast<unsigned char>(c));
			}
			break;
		}
	}
	return escaped.str();
}

Phasor::string CCodeGenerator::sanitizeModuleName(const Phasor::string &name)
{
	Phasor::string result;
	for (char c : name)
	{
		if ((std::isalnum(c) != 0) || c == '_')
		{
			result += c;
		} else {
			result += '_';
		}
	}

	// Ensure it starts with a letter or underscore
	if (!result.empty() && (std::isdigit(result[0]) != 0))
	{
		result = "_" + result;
	}

	return result.empty() ? "PhasorModule" : result;
}

} // namespace Phasor