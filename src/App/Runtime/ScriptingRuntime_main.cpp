#include "../../cli/Runtime/ScriptingRuntime.hpp"
#include "../../Frontend/Frontend.hpp"
#include <iostream>

int main(int argc, char *argv[], char *envp[])
{
	if (argc != 1)
		std::cout << "Phasor JIT Compiler / Runtime\n(C) 2025 Daniel McGuire\n\n";
	try
	{
		Phasor::ScriptingRuntime ScriptRT(argc, argv, envp);
		return ScriptRT.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		return 1;
	}

	return 0;
}