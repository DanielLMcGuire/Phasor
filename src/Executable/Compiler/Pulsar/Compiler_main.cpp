#include "../../../Compiler/Pulsar/Compiler.hpp"
#include "../../../Frontend/Pulsar/Frontend.hpp"
#include <nativeerror.h>

int main(int argc, char *argv[])
{
	try
	{
		pulsar::Compiler compiler(argc, argv);
		return compiler.run();
	}
	catch (const std::exception &e)
	{
		error(e.what());
		return 1;
	}

	return 0;
}
