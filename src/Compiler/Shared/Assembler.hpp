#pragma once

#include <string>
#include <filesystem>
/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/**
 * @class Assembler
 * @brief CLI wrapper for assembling Phasor binaries
 */
class Assembler
{
  public:
	Assembler(int argc, char *argv[]);
	int run();

  private:
	struct Args
	{
		std::filesystem::path inputFile;
		std::filesystem::path outputFile;
		std::filesystem::path program;
		bool                  noLogo = false;
		bool                  showHelp = false;
		bool				  silent = false;
	} m_args;

	bool parseArguments(int argc, char *argv[]);
	void showHelp();
	bool assembleBinary();
};

} // namespace Phasor
