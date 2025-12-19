#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
	/// @brief Native bitwise NOT
	int64_t asm_not(int64_t a);
	/// @brief Native bitwise AND
	int64_t asm_and(int64_t a, int64_t b);
	/// @brief Native bitwise OR
	int64_t asm_or(int64_t a, int64_t b);
	/// @brief Native bitwise XOR
	int64_t asm_xor(int64_t a, int64_t b);
	/// @brief Native Equality comparison
	int64_t asm_equal(int64_t a, int64_t b);
	/// @brief Native Inequality comparison
	int64_t asm_not_equal(int64_t a, int64_t b);
	/// @brief Native Less than comparison
	int64_t asm_less_than(int64_t a, int64_t b);
	/// @brief Native Greater than comparison
	int64_t asm_greater_than(int64_t a, int64_t b);
	/// @brief Native Less than or equal comparison
	int64_t asm_less_equal(int64_t a, int64_t b);
	/// @brief Native Greater than or equal comparison
	int64_t asm_greater_equal(int64_t a, int64_t b);
#ifdef __cplusplus
}
#endif