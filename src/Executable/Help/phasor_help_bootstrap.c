#include <PhasorRT.h>

int main(int argc, char *argv[])
{
	return exec(nullptr, embeddedBytecode, embeddedBytecodeSize, moduleName, argc, (const char **)argv);
}
