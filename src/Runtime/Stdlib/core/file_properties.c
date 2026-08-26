#include "file_properties.h"
#include <stdint.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #include <sddl.h>
    #include <aclapi.h>

    #define WIN_EPOCH_OFFSET 116444736000000000LL
    #define WIN_TICK_INTERVAL 10000000LL
#else
    #include <sys/stat.h>
    #include <unistd.h>
    #include <utime.h>
    #include <errno.h>
#endif

#ifdef _WIN32
[[nodiscard]] static FILETIME UnixTimeToFileTime(int64_t epoch) {
    long long ll = (epoch * WIN_TICK_INTERVAL) + WIN_EPOCH_OFFSET;
    FILETIME ft;
    ft.dwLowDateTime = (DWORD)ll;
    ft.dwHighDateTime = (DWORD)(ll >> 32);
    return ft;
}

[[nodiscard]] static int64_t FileTimeToUnixTime(FILETIME ft) {
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    return (int64_t)((ull.QuadPart - WIN_EPOCH_OFFSET) / WIN_TICK_INTERVAL);
}

[[nodiscard]] static uid_t HashSidToUint32(PSID pSid) {
    if (!pSid)
    {
        return 0;
    }

    PBYTE pBinarySid = (PBYTE)pSid;
    DWORD dwSidSize = GetLengthSid(pSid);
    uid_t hash = 5381;

    for (DWORD i = 0; i < dwSidSize; i++) {
        hash = ((hash << 5) + hash) + pBinarySid[i];
    }
    return hash;
}
#endif

[[nodiscard]] bool PHASORstd_file_setProperties(const char *path, char param, int64_t epoch) {
    if (!path) 
    { 
        return false;
    }

#ifdef _WIN32
    HANDLE hFile = CreateFileA(path, FILE_WRITE_ATTRIBUTES, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) 
    { 
        return false;
    }

    FILETIME ft = UnixTimeToFileTime(epoch);
    bool success = false;

    switch (param) {
        case 'a': success = (SetFileTime(hFile, nullptr, &ft, nullptr) != 0); break;
        case 'c': success = (SetFileTime(hFile, &ft, nullptr, nullptr) != 0); break;
        case 'm': success = (SetFileTime(hFile, nullptr, nullptr, &ft) != 0); break;
        default:  success = false; break;
    }

    CloseHandle(hFile);
    return success;
#else
    struct stat st;
    if (stat(path, &st) != 0) return false;

    struct utimbuf times;
    times.actime = st.st_atime;
    times.modtime = st.st_mtime;

    if (param == 'a') {
        times.actime = (time_t)epoch;
    } else if (param == 'm') {
        times.modtime = (time_t)epoch;
    } else {
        return false;
    }

    return utime(path, &times) == 0;
#endif
}

[[nodiscard]] int64_t PHASORstd_file_getProperties(const char *path, char param) {
    if (!path) 
    { 
        return -1;
    }

#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &fileInfo)) 
    { 
        return -1;
    }

    FILETIME ft;
    switch (param) {
        case 'a': ft = fileInfo.ftLastAccessTime; break;
        case 'c': ft = fileInfo.ftCreationTime; break;
        case 'm': ft = fileInfo.ftLastWriteTime; break;
        default:  return -1;
    }
    return FileTimeToUnixTime(ft);
#else
    struct stat st;
    if (stat(path, &st) != 0) return -1;

    if (param == 'a') return (int64_t)st.st_atime;
    if (param == 'm') return (int64_t)st.st_mtime;
#if defined(__APPLE__) || defined(__FreeBSD__)
    if (param == 'c') return (int64_t)st.st_birthtime;
#endif
    return -1;
#endif
}

[[nodiscard]] nlink_t PHASORstd_file_getLinksCount(const char *path) {
    if (!path) { return 0;
}

#ifdef _WIN32
    HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
                               nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) 
    { 
        return 0;
    }

    BY_HANDLE_FILE_INFORMATION info;
    nlink_t links = 0;
    if (GetFileInformationByHandle(hFile, &info)) {
        links = (nlink_t)info.nNumberOfLinks;
    }
    CloseHandle(hFile);
    return links;
#else
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return st.st_nlink;
#endif
}

[[nodiscard]] bool PHASORstd_file_getOwnerId(const char *path, uid_t *uid, gid_t *gid) {
    if (!path) 
    { 
        return false;
    }

#ifdef _WIN32
    PSID pSidOwner = nullptr;
    PSECURITY_DESCRIPTOR pSD = nullptr;

    if (GetNamedSecurityInfoA(path, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, 
                              &pSidOwner, nullptr, nullptr, nullptr, &pSD) != ERROR_SUCCESS) {
        return false;
    }

    if (uid) {
        *uid = HashSidToUint32(pSidOwner);
    }
    if (gid) {
        *gid = 0;
    }

    if (pSD) 
    { 
        LocalFree(pSD);
    }
    return true;
#else
    struct stat st;
    if (stat(path, &st) != 0) return false;
    if (uid) *uid = st.st_uid;
    if (gid) *gid = st.st_gid;
    return true;
#endif
}
