#include "../../../Runtime/Phasor/ScriptingRuntime.hpp"
#include "../../../Frontend/Phasor/Frontend.hpp"
#include <nativeerror.h>

int main(int argc, char *argv[], char *envp[])
{
	try
	{
		Phasor::ScriptingRuntime ScriptRT(argc, argv, envp);
		return ScriptRT.run();
	}
	catch (const std::exception &e)
	{
		error(e.what());
		return 1;
	}

	return 0;
}
