#include "../../../Compiler/Phasor/CppCompiler.hpp"

int main(int argc, char *argv[])
{
	Phasor::CppCompiler compiler(argc, argv);
	return compiler.run();
}
