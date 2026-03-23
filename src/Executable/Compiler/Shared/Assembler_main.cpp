#include "../../../Compiler/Shared/Assembler.hpp"
#include <nativeerror.h>

int main(int argc, char *argv[])
{
	try
	{
		Phasor::Assembler assemble(argc, argv);
		return assemble.run();
	}
	catch (const std::exception &e)
	{
		error(e.what());
		return 1;
	}

	return 0;
}
