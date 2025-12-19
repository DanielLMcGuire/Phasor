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
     * @brief Set file metadata time property
     * 
     * @param path Path to file
     * @param param What to change a=Access c=Creation m=Modified
     * @param epoch Epoch time to set
     * @return Status of operation
     */
    bool file_set_properties(char* path, char param, int64_t epoch);
	
    /**
	 * @brief Get file metadata time property
	 *
	 * @param path Path to file
	 * @param param What to get a=Access c=Creation m=Modified
	 * @returns epoch Epoch time
	 */
    int64_t file_get_properties(char* path, char param);

    /**
	 * @brief Retrieves the number of hard links to a file.
	 *
	 * This function returns the count of hard links associated with the specified file.
	 * On POSIX systems, it uses `stat` to get `st_nlink`. On Windows, it uses
	 * `GetFileInformationByHandle` to obtain `nNumberOfLinks`.
	 *
	 * @param path The path to the file.
	 * @return The number of hard links to the file. Returns 0 if the file cannot be accessed.
	 */
    nlink_t file_get_links_count(const char* path);

	/**
	 * @brief Retrieves the owner identifier of a file.
	 *
	 * On POSIX systems, this sets `*uid` to the file owner's UID and `*gid` to the file owner's GID.
	 * On Windows, there is no direct UID/GID, but a numeric representation of the owner SID is assigned
	 * to `*uid` and `*gid` is set to 0.
	 *
	 * @param path The path to the file.
	 * @param uid Pointer to a variable that receives the owner's UID.
	 * @param gid Pointer to a variable that receives the owner's GID.
	 * @return `true` if the owner information was successfully retrieved, `false` otherwise.
	 */
    bool file_get_owner_id(const char* path, uid_t* uid, gid_t* gid);
#ifdef __cplusplus
}
#endif