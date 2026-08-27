#pragma once
#include "../Bytecode.hpp"
#include <filesystem>
#include <sstream>
#include <PhasorString.hpp>
#include <phsint.hpp>

/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/**
 * @class CCodeGenerator
 * @brief Generates C++ header files with embedded Phasor bytecode
 *
 * This class takes compiled Phasor bytecode and generates a C++ header file
 * that embeds the bytecode as inline data. The header is designed to be
 * included in CppRuntime_main.cpp to provide the module name, bytecode array,
 * and bytecode size.
 */
class CCodeGenerator
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
	              const Phasor::string &moduleName = "");

	/**
	 * @brief Generate Bytecode object from embedded bytecode string
	 * @param input The string containing the embedded bytecode array
	 * @return Deserialized Bytecode object
	 *
	 */
	static Bytecode generateBytecodeFromEmbedded(const Phasor::string &input);

  private:
	std::ostringstream   output; ///< Output stream for generated code
	const Bytecode      *bytecode = nullptr;
	Phasor::string          moduleName;
	std::vector<u8> serializedBytecode; ///< Serialized bytecode in .phsb format

	// Code generation methods
	void generateFileHeader();
	void generateModuleName();
	void generateIncludes();
	void generateEmbeddedBytecode();
	void generateTempFileWriter();
	void generateMainFunction();

	// Deserialization helper
	static std::vector<unsigned char> parseEmbeddedBytecode(const Phasor::string &input);

	// Helper methods
	static Phasor::string escapeString(const Phasor::string &str);
	static Phasor::string getValueTypeString(ValueType type);
	static Phasor::string sanitizeModuleName(const Phasor::string &name);
};

} // namespace Phasor
