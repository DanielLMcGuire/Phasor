#include "../../../Compiler/Shared/Assembler.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
	try
	{
		Phasor::Assembler assemble(argc, argv);
		return assemble.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}

	return 0;
}