#include "../../../Compiler/Phasor/CCompiler.hpp"

int main(int argc, char *argv[])
{
	Phasor::CCompiler compiler(argc, argv);
	return compiler.run();
}
