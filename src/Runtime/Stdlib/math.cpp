#include "StdLib.hpp"
#include <phsint.hpp>

namespace Phasor
{

void StdLib::registerMathFunctions(VM *vm)
{
	vm->registerNativeFunction("math_sqrt", StdLib::math_sqrt);
	vm->registerNativeFunction("math_pow", StdLib::math_pow);
	vm->registerNativeFunction("math_abs", StdLib::math_abs);
	vm->registerNativeFunction("math_floor", StdLib::math_floor);
	vm->registerNativeFunction("math_ceil", StdLib::math_ceil);
	vm->registerNativeFunction("math_round", StdLib::math_round);
	vm->registerNativeFunction("math_min", StdLib::math_min);
	vm->registerNativeFunction("math_max", StdLib::math_max);
	vm->registerNativeFunction("math_log", StdLib::math_log);
	vm->registerNativeFunction("math_exp", StdLib::math_exp);
	vm->registerNativeFunction("math_sin", StdLib::math_sin);
	vm->registerNativeFunction("math_cos", StdLib::math_cos);
	vm->registerNativeFunction("math_tan", StdLib::math_tan);
}

f64 StdLib::math_sqrt(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "math_sqrt");
	if (!args[0].isNumber())
		PHS_ERROR("math_sqrt() expects a number as its argument");
	return asm_sqrt(args[0].asFloat());
}

f64 StdLib::math_pow(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "math_pow");
	if (!args[0].isNumber())
		PHS_ERROR("math_pow() expects a number as its first argument (base)");
	if (!args[1].isNumber())
		PHS_ERROR("math_pow() expects a number as its second argument (exponent)");
	f64 base = args[0].asFloat();
	f64 expv = args[1].asFloat();
	return asm_pow(base, expv);
}

Value StdLib::math_abs(const std::vector<Value> &args, VM *)
{
	/// @todo Implement abs natively
	checkArgCount(args, 1, "math_abs");
	if (!args[0].isNumber())
		PHS_ERROR("math_abs() expects a number as its argument");
	if (args[0].isInt()) 
	{
		return std::abs(args[0].asInt());
	}
	return std::abs(args[0].asFloat());
}

f64 StdLib::math_floor(const std::vector<Value> &args, VM *)
{
	/// @todo Implement floor natively
	checkArgCount(args, 1, "math_floor");
	if (!args[0].isNumber())
		PHS_ERROR("math_floor() expects a number as its argument");
	return std::floor(args[0].asFloat());
}

f64 StdLib::math_ceil(const std::vector<Value> &args, VM *)
{
	/// @todo Implement ceil natively
	checkArgCount(args, 1, "math_ceil");
	if (!args[0].isNumber())
		PHS_ERROR("math_ceil() expects a number as its argument");
	return std::ceil(args[0].asFloat());
}

f64 StdLib::math_round(const std::vector<Value> &args, VM *)
{
	/// @todo Implement round natively
	checkArgCount(args, 1, "math_round");
	if (!args[0].isNumber())
		PHS_ERROR("math_round() expects a number as its argument");
	return std::round(args[0].asFloat());
}

Value StdLib::math_min(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "math_min");
	const Value &a = args[0];
	const Value &b = args[1];
	if (a.isInt() && b.isInt())
	{
		i64 ai = a.asInt();
		i64 bi = b.asInt();
		return {(asm_iless_than(ai, bi) != 0) ? ai : bi};
	}
	if (a.isNumber() && b.isNumber())
	{
		f64 af = a.asFloat();
		f64 bf = b.asFloat();
		return {(asm_flless_than(af, bf) != 0) ? af : bf};
	}
	if (a.isString() && b.isString())
	{
		return a < b ? a : b;
	}
	PHS_ERROR("math_min() expects two numbers or two strings");
}

Value StdLib::math_max(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "math_max");
	const Value &a = args[0];
	const Value &b = args[1];
	if (a.isInt() && b.isInt())
	{
		i64 ai = a.asInt();
		i64 bi = b.asInt();
		return {ai > bi ? ai : bi};
	}
	if (a.isNumber() && b.isNumber())
	{
		f64 af = a.asFloat();
		f64 bf = b.asFloat();
		return {af > bf ? af : bf};
	}
	if (a.isString() && b.isString())
	{
		return a > b ? a : b;
	}
	PHS_ERROR("math_max() expects two numbers or two strings");
}

f64 StdLib::math_log(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "math_log");
	if (!args[0].isNumber())
		PHS_ERROR("math_log() expects a number as its argument");
	return asm_log(args[0].asFloat());
}

f64 StdLib::math_exp(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "math_exp");
	if (!args[0].isNumber())
		PHS_ERROR("math_exp() expects a number as its argument");
	return asm_exp(args[0].asFloat());
}

f64 StdLib::math_sin(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "math_sin");
	if (!args[0].isNumber())
		PHS_ERROR("math_sin() expects a number as its argument");
	return asm_sin(args[0].asFloat());
}

f64 StdLib::math_cos(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "math_cos");
	if (!args[0].isNumber())
		PHS_ERROR("math_cos() expects a number as its argument");
	return asm_cos(args[0].asFloat());
}

f64 StdLib::math_tan(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "math_tan");
	if (!args[0].isNumber())
		PHS_ERROR("math_tan() expects a number as its argument");
	return asm_tan(args[0].asFloat());
}
} // namespace Phasor
