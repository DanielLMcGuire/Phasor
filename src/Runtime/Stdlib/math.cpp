// Copyright 2025-2026 Daniel McGuire
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

f64 StdLib::math_sqrt(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "math_sqrt");
	requireNumber(args[0], "math_sqrt", "argument");
	return asm_sqrt(args[0].asFloat());
}

f64 StdLib::math_pow(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "math_pow");
	requireNumber(args[0], "math_pow", "first argument (base)");
	requireNumber(args[1], "math_pow", "second argument (exponent)");
	f64 base = args[0].asFloat();
	f64 expv = args[1].asFloat();
	return asm_pow(base, expv);
}

Value StdLib::math_abs(const Value::ArrayInstance &args, VM *)
{
	/// @todo Implement abs natively
	checkArgCount(args, 1, "math_abs");
	requireNumber(args[0], "math_abs", "argument");
	if (args[0].isInt()) 
	{
		return std::abs(args[0].asInt());
	}
	return std::abs(args[0].asFloat());
}

f64 StdLib::math_floor(const Value::ArrayInstance &args, VM *)
{
	/// @todo Implement floor natively
	checkArgCount(args, 1, "math_floor");
	requireNumber(args[0], "math_floor", "argument");
	return std::floor(args[0].asFloat());
}

f64 StdLib::math_ceil(const Value::ArrayInstance &args, VM *)
{
	/// @todo Implement ceil natively
	checkArgCount(args, 1, "math_ceil");
	requireNumber(args[0], "math_ceil", "argument");
	return std::ceil(args[0].asFloat());
}

f64 StdLib::math_round(const Value::ArrayInstance &args, VM *)
{
	/// @todo Implement round natively
	checkArgCount(args, 1, "math_round");
	requireNumber(args[0], "math_round", "argument");
	return std::round(args[0].asFloat());
}

Value StdLib::math_min(const Value::ArrayInstance &args, VM *)
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

Value StdLib::math_max(const Value::ArrayInstance &args, VM *)
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

f64 StdLib::math_log(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "math_log");
	requireNumber(args[0], "math_log", "argument");
	return asm_log(args[0].asFloat());
}

f64 StdLib::math_exp(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "math_exp");
	requireNumber(args[0], "math_exp", "argument");
	return asm_exp(args[0].asFloat());
}

f64 StdLib::math_sin(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "math_sin");
	requireNumber(args[0], "math_sin", "argument");
	return asm_sin(args[0].asFloat());
}

f64 StdLib::math_cos(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "math_cos");
	requireNumber(args[0], "math_cos", "argument");
	return asm_cos(args[0].asFloat());
}

f64 StdLib::math_tan(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "math_tan");
	requireNumber(args[0], "math_tan", "argument");
	return asm_tan(args[0].asFloat());
}
} // namespace Phasor
