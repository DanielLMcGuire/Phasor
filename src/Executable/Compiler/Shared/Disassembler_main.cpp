#include "../../../Compiler/Shared/Disassembler.hpp"
#include <nativeerror.h>

int main(int argc, char *argv[])
{
	try
	{
		Phasor::Disassembler disasm(argc, argv);
		return disasm.run();
	}
	catch (const std::exception &e)
	{
		error(e.what());
		return 1;
	}

	return 0;
}
