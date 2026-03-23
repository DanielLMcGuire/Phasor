#include "../../../Runtime/Shared/BinaryRuntime.hpp"
#include <nativeerror.h>

int main(int argc, char *argv[], char *envp[])
{
	try
	{
		Phasor::BinaryRuntime BinRT(argc, argv, envp);
		return BinRT.run();
	}
	catch (const std::exception &e)
	{
		error(e.what());
		return 1;
	}

	return 0;
}
