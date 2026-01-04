#include "../../CLI/Compiler/Compiler.hpp"
#include "../../Frontend/Frontend.hpp"
#include <iostream>

int main(int argc, char *argv[], char *envp[])
{
	std::cout << "Phasor Compiler\n(C) 2025 Daniel McGuire\n\n";
	try
	{
		Phasor::Compiler compiler(argc, argv, envp);
		return compiler.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}

	return 0;
}