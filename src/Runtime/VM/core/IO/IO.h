#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
	/// @brief Native print function
	void c_print_stdout(const char *s, int64_t len);
	/// @brief Native print error function
	void c_print_stderr(const char *s, int64_t len);
	/// @brief CRT system call
	int64_t c_system(const char *cmd);
	/// @brief CRT system call, get out
	char *c_system_out(const char *cmd);
	/// @brief CRT system call, get err
	char *c_system_err(const char *cmd);
#ifdef __cplusplus
}
#endif