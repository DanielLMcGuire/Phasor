#ifdef _WIN32
#include <Windows.h>
#define error(msg) \
	MessageBoxA(NULL, std::string(msg).c_str(), "Phasor VM Runtime Error", MB_OK | MB_ICONERROR); \
	std::cerr << "Error: " << msg << std::endl
#else
#define error(msg) std::cerr << "Error: " << msg << std::endl
#endif