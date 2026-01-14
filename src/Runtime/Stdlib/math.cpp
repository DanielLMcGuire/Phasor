#include "Runtime/Value.hpp"
#include "StdLib.hpp"

Value StdLib::registerMathFunctions(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "include_stdmath");
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
	return true;
}

Value StdLib::math_sqrt(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "math_sqrt");
	return Value(asm_sqrt(args[0].asFloat()));
}

Value StdLib::math_pow(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 2, "math_pow");
	double base = args[0].asFloat();
	double expv = args[1].asFloat();
	return Value(asm_pow(base, expv));
}

Value StdLib::math_abs(const std::vector<Value> &args, VM *vm)
{
	// @TODO: Implement abs natively
	checkArgCount(args, 1, "math_abs");
	if (args[0].isInt())
		return std::abs(args[0].asInt());
	return std::abs(args[0].asFloat());
}

Value StdLib::math_floor(const std::vector<Value> &args, VM *vm)
{
	// @TODO: Implement floor natively
	checkArgCount(args, 1, "math_floor");
	return static_cast<int64_t>(std::floor(args[0].asFloat()));
}

Value StdLib::math_ceil(const std::vector<Value> &args, VM *vm)
{
	// @TODO: Implement ceil natively
	checkArgCount(args, 1, "math_ceil");
	return static_cast<int64_t>(std::ceil(args[0].asFloat()));
}

Value StdLib::math_round(const std::vector<Value> &args, VM *vm)
{
	// @TODO: Implement round natively
	checkArgCount(args, 1, "math_round");
	return static_cast<int64_t>(std::round(args[0].asFloat()));
}

Value StdLib::math_min(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 2, "math_min");
	const Value &a = args[0];
	const Value &b = args[1];
	if (a.isInt() && b.isInt())
	{
		int64_t ai = a.asInt();
		int64_t bi = b.asInt();
		return Value(asm_iless_than(ai, bi) ? ai : bi);
	}
	if (a.isNumber() && b.isNumber())
	{
		double af = a.asFloat();
		double bf = b.asFloat();
		return Value(asm_flless_than(af, bf) ? af : bf);
	}
	return a < b ? a : b;
}

Value StdLib::math_max(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 2, "math_max");
	const Value &a = args[0];
	const Value &b = args[1];
	if (a.isInt() && b.isInt())
	{
		int64_t ai = a.asInt();
		int64_t bi = b.asInt();
		return Value(ai > bi ? ai : bi);
	}
	if (a.isNumber() && b.isNumber())
	{
		double af = a.asFloat();
		double bf = b.asFloat();
		return Value(af > bf ? af : bf);
	}
	return a > b ? a : b;
}

Value StdLib::math_log(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "math_log");
	return Value(asm_log(args[0].asFloat()));
}

Value StdLib::math_exp(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "math_exp");
	return Value(asm_exp(args[0].asFloat()));
}

Value StdLib::math_sin(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "math_sin");
	return Value(asm_sin(args[0].asFloat()));
}

Value StdLib::math_cos(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "math_cos");
	return Value(asm_cos(args[0].asFloat()));
}

Value StdLib::math_tan(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "math_tan");
	return Value(asm_tan(args[0].asFloat()));
}