#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
	/// @brief Native bitwise NOT
	int64_t asm_flnot(double a);
	/// @brief Native bitwise AND
	int64_t asm_iand(int64_t a, int64_t b);
	int64_t asm_fland(double a, double b);
	/// @brief Native bitwise OR
	int64_t asm_ior(int64_t a, int64_t b);
	int64_t asm_flor(double a, double b);
	/// @brief Native bitwise XOR
	int64_t asm_ixor(int64_t a, int64_t b);
	int64_t asm_flxor(double a, double b);
	/// @brief Native Equality comparison
	int64_t asm_iequal(int64_t a, int64_t b);
	int64_t asm_flequal(double a, double b);
	/// @brief Native Inequality comparison
	int64_t asm_inot_equal(int64_t a, int64_t b);
	int64_t asm_flnot_equal(double a, double b);
	/// @brief Native Less than comparison
	int64_t asm_iless_than(int64_t a, int64_t b);
	int64_t asm_flless_than(double a, double b);
	/// @brief Native Greater than comparison
	int64_t asm_igreater_than(int64_t a, int64_t b);
	int64_t asm_flgreater_than(double a, double b);
	/// @brief Native Less than or equal comparison
	int64_t asm_iless_equal(int64_t a, int64_t b);
	int64_t asm_flless_equal(double a, double b);
	/// @brief Native Greater than or equal comparison
	int64_t asm_igreater_equal(int64_t a, int64_t b);
	int64_t asm_flgreater_equal(double a, double b);
#ifdef __cplusplus
}
#endif