#pragma once
#include "../CodeGen.hpp"
#include <filesystem>
#include <sstream>
#include <string>

namespace Phasor
{

/**
 * @class CppCodeGenerator
 * @brief Generates C++ header files with embedded Phasor bytecode
 *
 * This class takes compiled Phasor bytecode and generates a C++ header file
 * that embeds the bytecode as inline data. The header is designed to be
 * included in CppRuntime_main.cpp to provide the module name, bytecode array,
 * and bytecode size.
 */
class CppCodeGenerator
{
  public:
	/**
	 * @brief Generate C++ header file from bytecode
	 *
	 * @param bytecode The compiled bytecode to embed
	 * @param outputPath Path to the output header file
	 * @param moduleName Optional module name (defaults to filename without extension)
	 * @return true if generation succeeded, false otherwise
	 */
	bool generate(const Bytecode &bytecode, const std::filesystem::path &outputPath,
	              const std::string &moduleName = "");

	/**
	 * @brief Generate Bytecode object from embedded bytecode string
	 * @param input The string containing the embedded bytecode array
	 * @return Deserialized Bytecode object
	 *
	 */
	Bytecode generateBytecodeFromEmbedded(const std::string &input);

  private:
	std::ostringstream   output; ///< Output stream for generated code
	const Bytecode      *bytecode = nullptr;
	std::string          moduleName;
	std::vector<uint8_t> serializedBytecode; ///< Serialized bytecode in .phsb format

	// Code generation methods
	void generateFileHeader();
	void generateModuleName();
	void generateIncludes();
	void generateEmbeddedBytecode();
	void generateTempFileWriter();
	void generateMainFunction();

	// Deserialization helper
	std::vector<unsigned char> parseEmbeddedBytecode(const std::string &input);

	// Helper methods
	std::string escapeString(const std::string &str);
	std::string getValueTypeString(ValueType type);
	std::string sanitizeModuleName(const std::string &name);
};

} // namespace Phasor
