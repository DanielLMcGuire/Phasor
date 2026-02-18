#include "../../../Compiler/Shared/Disassembler.hpp"
#include <iostream>

int main(int argc, char *argv[], char *envp[])
{
	try
	{
		Phasor::Disassembler disasm(argc, argv);
		return disasm.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}

	return 0;
}