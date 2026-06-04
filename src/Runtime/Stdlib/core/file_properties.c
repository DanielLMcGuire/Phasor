#include "file_properties.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
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
static FILETIME UnixTimeToFileTime(int64_t epoch) {
    long long ll = (epoch * WIN_TICK_INTERVAL) + WIN_EPOCH_OFFSET;
    FILETIME ft;
    ft.dwLowDateTime = (DWORD)ll;
    ft.dwHighDateTime = (DWORD)(ll >> 32);
    return ft;
}

static int64_t FileTimeToUnixTime(FILETIME ft) {
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    return (int64_t)((ull.QuadPart - WIN_EPOCH_OFFSET) / WIN_TICK_INTERVAL);
}

/**
 * Deterministically hashes a Windows SID into a 32-bit integer.
 */
static uid_t HashSidToUint32(PSID pSid) {
    if (!pSid) return 0;

    PBYTE pBinarySid = (PBYTE)pSid;
    DWORD dwSidSize = GetLengthSid(pSid);
    uid_t hash = 5381; // djb2 hash algorithm

    for (DWORD i = 0; i < dwSidSize; i++) {
        hash = ((hash << 5) + hash) + pBinarySid[i];
    }
    return hash;
}
#endif

bool PHASORstd_file_setProperties(const char *path, char param, int64_t epoch) {
    if (!path) return false;

#ifdef _WIN32
    HANDLE hFile = CreateFileA(path, FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    FILETIME ft = UnixTimeToFileTime(epoch);
    bool success = false;

    switch (param) {
        case 'a': success = (SetFileTime(hFile, NULL, &ft, NULL) != 0); break;
        case 'c': success = (SetFileTime(hFile, &ft, NULL, NULL) != 0); break;
        case 'm': success = (SetFileTime(hFile, NULL, NULL, &ft) != 0); break;
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
        return false; // Creation time not settable via standard POSIX utime
    }

    return utime(path, &times) == 0;
#endif
}

int64_t PHASORstd_file_getProperties(const char *path, char param) {
    if (!path) return -1;

#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &fileInfo)) return -1;

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

nlink_t PHASORstd_file_getLinksCount(const char *path) {
    if (!path) return 0;

#ifdef _WIN32
    HANDLE hFile = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
                               NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return 0;

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

bool PHASORstd_file_getOwnerId(const char *path, uid_t *uid, gid_t *gid) {
    if (!path) return false;

#ifdef _WIN32
    PSID pSidOwner = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;

    if (GetNamedSecurityInfoA(path, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, 
                              &pSidOwner, NULL, NULL, NULL, &pSD) != ERROR_SUCCESS) {
        return false;
    }

    if (uid) {
        *uid = HashSidToUint32(pSidOwner);
    }
    if (gid) {
        *gid = 0;
    }

    if (pSD) LocalFree(pSD);
    return true;
#else
    struct stat st;
    if (stat(path, &st) != 0) return false;
    if (uid) *uid = st.st_uid;
    if (gid) *gid = st.st_gid;
    return true;
#endif
}