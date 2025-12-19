#include <stdint.h>
#include <stdbool.h>
#include "file_properties.h"

#ifdef _WIN32
#include <windows.h>
#include <fileapi.h>
#include <handleapi.h>
#include <winnt.h>
#include <Aclapi.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include <errno.h>
#include <string.h>
#endif


bool file_set_properties(char* path, char param, int64_t epoch) {
#ifdef _WIN32
    HANDLE hFile = CreateFileA(path, FILE_WRITE_ATTRIBUTES, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    FILETIME ft;
    LONGLONG ll = Int32x32To64(epoch, 10000000) + 116444736000000000; // convert epoch to FILETIME
    ft.dwLowDateTime = (DWORD)ll;
    ft.dwHighDateTime = (DWORD)(ll >> 32);

    bool result = false;
    if (param == 'a') {
        result = SetFileTime(hFile, NULL, &ft, NULL) != 0;
    } else if (param == 'c') {
        result = SetFileTime(hFile, &ft, NULL, NULL) != 0;
    } else if (param == 'm') {
        result = SetFileTime(hFile, NULL, NULL, &ft) != 0;
    }

    CloseHandle(hFile);
    return result;

#else
    struct stat st;
    if (stat(path, &st) != 0) return false;

    struct utimbuf times;
    times.actime = st.st_atime;
    times.modtime = st.st_mtime;

    if (param == 'a') {
        times.actime = epoch;
    } else if (param == 'm') {
        times.modtime = epoch;
    } else {
        return false; // POSIX doesn't support setting creation time
    }

    return utime(path, &times) == 0;
#endif
}

int64_t file_get_properties(char* path, char param) {
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &fileInfo)) return -1;

    FILETIME ft;
    if (param == 'a') ft = fileInfo.ftLastAccessTime;
    else if (param == 'c') ft = fileInfo.ftCreationTime;
    else if (param == 'm') ft = fileInfo.ftLastWriteTime;
    else return -1;

    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    return (int64_t)((ull.QuadPart - 116444736000000000) / 10000000);

#else
    struct stat st;
    if (stat(path, &st) != 0) return -1;

    if (param == 'a') return (int64_t)st.st_atime;
    else if (param == 'm') return (int64_t)st.st_mtime;
#if defined(__APPLE__) || defined(__FreeBSD__)
    else if (param == 'c') return (int64_t)st.st_birthtime; // BSD creation time
#endif
    else return -1; // creation time not supported on other POSIX systems
#endif
}

nlink_t file_get_links_count(const char* path) 
#ifdef _WIN32
{

    HANDLE hFile = CreateFileA(path, GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                               NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return 0;

    BY_HANDLE_FILE_INFORMATION info;
    if (!GetFileInformationByHandle(hFile, &info)) {
        CloseHandle(hFile);
        return 0;
    }
    CloseHandle(hFile);
    return (nlink_t)info.nNumberOfLinks;
}
#else
nlink_t file_get_links_count(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return st.st_nlink;
}
#endif


bool file_get_owner_id(const char* path, uid_t* uid, gid_t* gid) 
#ifdef _WIN32
{
    // Windows does not have UID/GID like POSIX, but we can get the owner SID
    *uid = 0;
    *gid = 0;

    PSID pSidOwner = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    if (GetNamedSecurityInfoA(path, SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION,
                              &pSidOwner, NULL, NULL, NULL, &pSD) != ERROR_SUCCESS) {
        return false;
    }

    // Optional: convert SID to some numeric representation
    *uid = (uid_t)GetSidIdentifierAuthority(pSidOwner)->Value[5]; // crude
    *gid = 0;

    if (pSD) LocalFree(pSD);
    return true;
}
#else // POSIX
{
    struct stat st;
    if (stat(path, &st) != 0) return false;
    if (uid) *uid = st.st_uid;
    if (gid) *gid = st.st_gid;
    return true;
}
#endif
