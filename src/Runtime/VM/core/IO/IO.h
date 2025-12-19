#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
	/// @brief Native print function
	void asm_print_stdout(const char *s, int64_t len);
	/// @brief Native print error function
	void asm_print_stderr(const char *s, int64_t len);
	/// @brief CRT system call
	int64_t asm_system(const char *cmd);
#ifdef __cplusplus
}
#endif