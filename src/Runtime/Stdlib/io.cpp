#include "StdLib.hpp"
#include <vformat.hpp>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <cstdlib>

namespace Phasor
{

void StdLib::registerIOFunctions(VM *vm)
{
	vm->registerNativeFunction("c_fmt", StdLib::io_c_format);
	vm->registerNativeFunction("printf", StdLib::io_printf);
	vm->registerNativeFunction("putf", StdLib::io_putf);
	vm->registerNativeFunction("print_error", StdLib::io_print_error);
	vm->registerNativeFunction("putf_error", StdLib::io_putf_error);
#ifndef SANDBOXED
	vm->registerNativeFunction("clear", StdLib::io_clear);
	vm->registerNativeFunction("gets", StdLib::io_gets);
	vm->registerNativeFunction("get_input", StdLib::io_get_input);
#endif
}

#ifndef SANDBOXED
Value StdLib::io_clear(const Value::ArrayInstance &args, VM *vm)
{
	checkArgCount(args, 0, "clear");
	vm->regRun(OpCode::PRINT_R, "\033[2J\033[H");
	return phsnull;
}
#endif

Phasor::string StdLib::io_c_format(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "c_fmt", true);
	if (args.empty())
	{
		return ""; // Return empty string if no arguments
	}

	requireString(args[0], "c_fmt", "format");

	const Phasor::string &fmt = args[0].string();

	// Make vector of format args
	Value::ArrayInstance formatArgs(args.begin() + 1, args.end());

	return vformat::str_format_v(fmt.c_str(), formatArgs);
	
}

Phasor::string StdLib::io_printf(const Value::ArrayInstance &args, VM *vm)
{
	checkArgCount(args, 1, "printf", true);
	requireString(args[0], "printf", "format");

	Value::ArrayInstance formatArgs(args.begin(), args.end());
	vm->regRun(OpCode::PRINT_R, io_c_format(formatArgs, vm));
	return "";
}

Phasor::string StdLib::io_putf(const Value::ArrayInstance &args, VM *vm)
{
	checkArgCount(args, 1, "putf", true);
	requireString(args[0], "putf", "format");

	Value::ArrayInstance formatArgs(args.begin(), args.end());
	Phasor::string        input = io_c_format(formatArgs, vm);
	vm->regRun(OpCode::PRINT_R, input.str() + "\n");
	return "";
}

#ifndef SANDBOXED
Value StdLib::io_gets(const Value::ArrayInstance &args, VM *vm)
{
	checkArgCount(args, 0, "gets");
	return vm->regRun(OpCode::READLINE_R, REGISTER1);
}

Value StdLib::io_get_input(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 0, "get_input");
	std::string content((std::istreambuf_iterator<char>(std::cin)), std::istreambuf_iterator<char>());
	return content;
}
#endif

Phasor::string StdLib::io_print_error(const Value::ArrayInstance &args, VM *vm)
{
	checkArgCount(args, 1, "print_error");
	Phasor::string input = args[0].toString();
	vm->regRun(OpCode::PRINTERROR_R, input);
	return "";
}

Phasor::string StdLib::io_putf_error(const Value::ArrayInstance &args, VM *vm)
{
	checkArgCount(args, 1, "putf_error", true);
	requireString(args[0], "putf_error", "format");
	Value::ArrayInstance formatArgs(args.begin(), args.end());
	Phasor::string input = io_c_format(formatArgs, vm);
	vm->regRun(OpCode::PRINTERROR_R, input + "\n");
	return "";
}
} // namespace Phasor
