#include <print>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#define error(msg) \
	MessageBoxA(NULL, std::string(msg).c_str(), "Phasor VM Runtime Error", MB_OK | MB_ICONERROR); \
	std::println(std::cerr, "Error: {}", msg)
#else
#define error(msg) std::println(std::cerr, "Error: {}", msg)
#endif
