#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
	/// @brief Native addition
	int64_t asm_add(int64_t a, int64_t b);
	/// @brief Native subtraction
	int64_t asm_sub(int64_t a, int64_t b);
	/// @brief Native multiplication
	int64_t asm_mul(int64_t a, int64_t b);
	/// @brief Native negation
	int64_t asm_neg(int64_t a);
	/// @brief Native division
	int64_t asm_div(int64_t a, int64_t b);
	/// @brief Native modulus
	int64_t asm_mod(int64_t a, int64_t b);
	/// @brief Native square root
	double asm_sqrt(double a);
	/// @brief Native power
	double asm_pow(double a, double b);
	/// @brief Native natural logarithm
	double asm_log(double a);
	/// @brief Native exponential
	double asm_exp(double a);
	/// @brief Native sine
	double asm_sin(double a);
	/// @brief Native cosine
	double asm_cos(double a);
	/// @brief Native tangent
	double asm_tan(double a);
#ifdef __cplusplus
}
#endif