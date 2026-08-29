// Copyright 2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://phasor.pages.dev/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include <print>
#include <iostream>
#include <PhasorString.hpp>

#define _phs_nativeerror_console(msg) std::println(std::cerr, "Error: {}", msg)

#define error(msg) _phs_nativeerror_console(msg)

#if defined(_DEBUG)

#if defined(_WIN32)
#include <Windows.h>
#undef error
#define error(msg)                                                                                                     \
	do                                                                                                                 \
	{                                                                                                                  \
		Phasor::string _msg = (msg);                                                                                      \
		MessageBoxA(nullptr, _msg.c_str(), "Phasor VM Runtime Error", MB_OK | MB_ICONERROR);                              \
		_phs_nativeerror_console(_msg);                                                                                \
	} while (0)
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#undef error
#define error(msg)                                                                                                     \
	do                                                                                                                 \
	{                                                                                                                  \
		Phasor::string _msg = (msg);                                                                                      \
		CFStringRef _cfMsg = CFStringCreateWithCString(nullptr, _msg.c_str(), kCFStringEncodingUTF8);                     \
		CFUserNotificationDisplayAlert(0, kCFUserNotificationStopAlertLevel, nullptr, nullptr, nullptr,                         \
		                               CFSTR("Phasor VM Runtime Error"), _cfMsg, CFSTR("OK"), nullptr, nullptr, nullptr);       \
		CFRelease(_cfMsg);                                                                                             \
		_phs_nativeerror_console(_msg);                                                                                \
	} while (0)
#endif

#endif