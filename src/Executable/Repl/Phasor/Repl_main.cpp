#include "../../../Frontend/Phasor/Frontend.hpp"
#include <iostream>

int main(int argc, char *argv[], char *envp[])
{
    try
    {
		return Phasor::Frontend::runRepl();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}