#include "../../../CLI/Pulsar/Interpreter/Interpreter.hpp"
#include "../../../Frontend/Pulsar/Frontend.hpp"
#include <iostream>

int main(int argc, char *argv[], char *envp[])
{
	try
	{
		if (argc >= 2)
		{
			pulsar::Interpreter interp(argc, argv, envp);
			return interp.run();
		}
		else
		{
			
			pulsar::Frontend::runRepl();
		}
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}

	return 0;
}