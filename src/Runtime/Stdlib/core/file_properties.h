#ifndef FILE_PROPERTIES_H
#define FILE_PROPERTIES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
    typedef unsigned long nlink_t;
    typedef unsigned long uid_t;
    typedef unsigned long gid_t;
#else
    #include <sys/types.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Set file metadata time property.
     * @param path Path to file.
     * @param param Property to change: 'a' = Access, 'c' = Creation, 'm' = Modified.
     * @param epoch Epoch time to set.
     * @return true if successful, false otherwise.
     */
    bool PHASORstd_file_setProperties(const char *path, char param, int64_t epoch);

    /**
     * @brief Get file metadata time property.
     * @param path Path to file.
     * @param param Property to retrieve: 'a' = Access, 'c' = Creation, 'm' = Modified.
     * @return Epoch time, or -1 on failure.
     */
    int64_t PHASORstd_file_getProperties(const char *path, char param);

    /**
     * @brief Retrieves the number of hard links to a file.
     * @param path The path to the file.
     * @return The number of hard links. Returns 0 if the file cannot be accessed.
     */
    nlink_t PHASORstd_file_getLinksCount(const char *path);

    /**
     * @brief Retrieves the owner identifier of a file.
     * @param path The path to the file.
     * @param uid Pointer to receive the owner's UID (Deterministic hash of SID on Windows).
     * @param gid Pointer to receive the owner's GID.
     * @return true if information was successfully retrieved, false otherwise.
     */
    bool PHASORstd_file_getOwnerId(const char *path, uid_t *uid, gid_t *gid);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // FILE_PROPERTIES_H
