#include <string>
#include <sstream>

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