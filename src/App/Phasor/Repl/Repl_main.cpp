#include "../../../CLI/Phasor/Repl/Repl.hpp"
#include "../../../Frontend/Phasor/Frontend.hpp"
#include <iostream>

int main(int argc, char *argv[], char *envp[])
{
	try
	{
		Phasor::Repl Repl(argc, argv, envp);
		return Repl.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}

	return 0;
}