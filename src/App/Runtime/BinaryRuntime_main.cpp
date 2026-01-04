#include "../../CLI/Runtime/BinaryRuntime.hpp"

#include <iostream>

int main(int argc, char *argv[], char *envp[])
{
	std::cout << "Phasor VM Runtime\n(C) 2025 Daniel McGuire\n\n";
	try
	{
		Phasor::BinaryRuntime BinRT(argc, argv, envp);
		return BinRT.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}

	return 0;
}