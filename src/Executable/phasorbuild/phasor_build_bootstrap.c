#include <PhasorRT.h>

typedef void* VM;

int main(int argc, char *argv[])
{
	VMState vm = createState();
	initStdLib(vm);
	int ret = exec(vm, embeddedBytecode, embeddedBytecodeSize, moduleName, argc, (const char **)argv);
	freeState(vm);
	return ret;
}
