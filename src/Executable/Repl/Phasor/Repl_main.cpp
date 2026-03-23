#include "../../../Frontend/Phasor/Frontend.hpp"
#include <nativeerror.h>

int main(int argc, char *argv[], char *envp[])
{
    try
    {
		return Phasor::Frontend::runRepl();
    }
    catch (const std::exception &e)
    {
        error(e.what());
        return 1;
    }
}
