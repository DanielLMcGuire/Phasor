#pragma once
#include <print>
#include <iostream>
#include <string>

#define _phs_nativeerror_console(msg) std::println(std::cerr, "Error: {}", msg)

#define error(msg) _phs_nativeerror_console(msg)

#if defined(_DEBUG)

#if defined(_WIN32)
#include <Windows.h>
#undef error
#define error(msg)                                                                                                     \
	do                                                                                                                 \
	{                                                                                                                  \
		std::string _msg = (msg);                                                                                      \
		MessageBoxA(nullptr, _msg.c_str(), "Phasor VM Runtime Error", MB_OK | MB_ICONERROR);                              \
		_phs_nativeerror_console(_msg);                                                                                \
	} while (0)
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#undef error
#define error(msg)                                                                                                     \
	do                                                                                                                 \
	{                                                                                                                  \
		std::string _msg = (msg);                                                                                      \
		CFStringRef _cfMsg = CFStringCreateWithCString(nullptr, _msg.c_str(), kCFStringEncodingUTF8);                     \
		CFUserNotificationDisplayAlert(0, kCFUserNotificationStopAlertLevel, nullptr, nullptr, nullptr,                         \
		                               CFSTR("Phasor VM Runtime Error"), _cfMsg, CFSTR("OK"), nullptr, nullptr, nullptr);       \
		CFRelease(_cfMsg);                                                                                             \
		_phs_nativeerror_console(_msg);                                                                                \
	} while (0)
#endif

#endif