#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
	/// @brief Native addition
	int64_t asm_iadd(int64_t a, int64_t b);
	double  asm_fladd(double a, double b);
	/// @brief Native subtraction
	int64_t asm_isub(int64_t a, int64_t b);
	double  asm_flsub(double a, double b);
	/// @brief Native multiplication
	int64_t asm_imul(int64_t a, int64_t b);
	double  asm_flmul(double a, double b);
	/// @brief Native negation
	int64_t asm_flneg(double a);
	/// @brief Native division
	int64_t asm_idiv(int64_t a, int64_t b);
	double  asm_fldiv(double a, double b);
	/// @brief Native modulus
	int64_t asm_imod(int64_t a, int64_t b);
	double  asm_flmod(double a, double b);
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