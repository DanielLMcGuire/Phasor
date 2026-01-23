#include "../../CLI/Runtime/ScriptingRuntime.hpp"
#include "../../Frontend/Frontend.hpp"
#include <iostream>

int main(int argc, char *argv[], char *envp[])
{
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