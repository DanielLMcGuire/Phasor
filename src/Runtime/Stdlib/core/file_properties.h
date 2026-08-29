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

#ifndef FILE_PROPERTIES_H
#define FILE_PROPERTIES_H

// Phasor stdlibcore file/properties

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
