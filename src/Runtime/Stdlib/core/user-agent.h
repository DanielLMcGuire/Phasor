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

#include <string>
#include <sstream>

// Phasor stdlibcore http/user-agent


#if defined(_WIN32)
    #include <windows.h>
    #include <winternl.h>

#elif defined(__APPLE__)
    #include <sys/types.h>
    #include <sys/sysctl.h>

#else
    #include <sys/utsname.h>
#endif

inline std::string getUserAgentOS()
{
#if defined(_WIN32)
    using RtlGetVersionFn = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);

    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (!ntdll)
        return "(Windows)";

    auto RtlGetVersion =
        reinterpret_cast<RtlGetVersionFn>(
            GetProcAddress(ntdll, "RtlGetVersion"));

    if (!RtlGetVersion)
        return "(Windows)";

    RTL_OSVERSIONINFOW version{};
    version.dwOSVersionInfoSize = sizeof(version);

    if (RtlGetVersion(&version) != 0)
        return "(Windows)";

    std::ostringstream os;

    os << "(Windows NT "
       << version.dwMajorVersion
       << "."
       << version.dwMinorVersion;

#if defined(_WIN64)
    os << "; Win64; x64";
#else
    os << "; Win32; x86";
#endif

    os << ")";

    return os.str();

#elif defined(__APPLE__)
    char version[256]{};
    size_t size = sizeof(version);

    if (sysctlbyname("kern.osproductversion",
                     version,
                     &size,
                     nullptr,
                     0) != 0)
    {
        return "(Macintosh)";
    }

    std::string macVersion(version);

    for (char& c : macVersion)
    {
        if (c == '.')
            c = '_';
    }

#if defined(__aarch64__) || defined(__arm64__)
    return "(Macintosh; ARM Mac OS X " + macVersion + ")";
#elif defined(__x86_64__)
    return "(Macintosh; Intel Mac OS X " + macVersion + ")";
#else
    return "(Macintosh; Mac OS X " + macVersion + ")";
#endif

#else

    struct utsname info{};

    if (uname(&info) != 0)
        return "(X11)";

    std::string sysname(info.sysname);
    std::string release(info.release);
    std::string machine(info.machine);

    if (sysname == "FreeBSD" ||
        sysname == "OpenBSD" ||
        sysname == "NetBSD" ||
        sysname == "DragonFly")
    {
        return "(X11; " + sysname + " " + machine + ")";
    }

    if (sysname == "Linux")
    {
        return "(X11; Linux " + machine + ")";
    }

    return "(X11; " + sysname + " " + machine + ")";

#endif
}