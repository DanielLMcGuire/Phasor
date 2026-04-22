#include "../../../Runtime/Shared/BinaryRuntime.hpp"
#include <nativeerror.h>

int main(int argc, char *argv[])
{
	try
	{
		Phasor::BinaryRuntime BinRT(argc, argv);
		return BinRT.run();
	}
	catch (const std::exception &e)
	{
		error(e.what());
		return 1;
	}

	return 0;
}
