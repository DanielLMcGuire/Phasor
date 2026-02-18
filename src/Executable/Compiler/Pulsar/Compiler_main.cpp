#include "../../../Compiler/Pulsar/Compiler.hpp"
#include "../../../Frontend/Pulsar/Frontend.hpp"
#include <iostream>

int main(int argc, char *argv[], char *envp[])
{
	try
	{
		pulsar::Compiler compiler(argc, argv, envp);
		return compiler.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}

	return 0;
}