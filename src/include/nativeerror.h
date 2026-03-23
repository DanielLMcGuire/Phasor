#include <cstdio>

#ifdef _WIN32
#include <Windows.h>
#define error(msg) \
	MessageBoxA(NULL, std::string(msg).c_str(), "Phasor VM Runtime Error", MB_OK | MB_ICONERROR); \
	fprintf(stderr, "Error: %s\n", msg)
#else
#define error(msg) fprintf(stderr, "Error: %s\n", std::string(msg).c_str())
#endif
