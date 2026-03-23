#include "../../../Compiler/Phasor/Compiler.hpp"
#include "../../../Frontend/Phasor/Frontend.hpp"
#include <nativeerror.h>

int main(int argc, char *argv[], char *envp[])
{
	try
	{
		Phasor::Compiler compiler(argc, argv, envp);
		return compiler.run();
	}
	catch (const std::exception &e)
	{
		error(e.what());
		return 1;
	}

	return 0;
}
