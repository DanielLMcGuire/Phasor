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
	vm->registerNativeFunction("puts_error", StdLib::io_puts_error);
	vm->registerNativeFunction("print_error", StdLib::io_print_error);
	vm->registerNativeFunction("putf_error", StdLib::io_putf_error);
#ifndef SANDBOXED
	vm->registerNativeFunction("clear", StdLib::io_clear);
	vm->registerNativeFunction("gets", StdLib::io_gets);
	vm->registerNativeFunction("get_input", StdLib::io_get_input);
#endif
}

#ifndef SANDBOXED
Value StdLib::io_clear(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "clear");
	vm->regRun(OpCode::PRINT_R, "\033[2J\033[H");
	return phsnull;
}
#endif

PhsString StdLib::io_c_format(const std::vector<Value> &args, VM *)
{
	if (args.empty())
	{
		return ""; // Return empty string if no arguments
	}

	if (!args[0].isString())
		PHS_ERROR("c_fmt() expects a string as its first argument (format)");

	const PhsString &fmt = args[0].string();

	// Make vector of format args
	std::vector<Value> formatArgs(args.begin() + 1, args.end());

	return vformat::str_format_v(fmt.c_str(), formatArgs);
	
}

PhsString StdLib::io_printf(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "printf", true);
	if (!args[0].isString())
		PHS_ERROR("printf() expects a string as its first argument (format)");
	std::vector<Value> formatArgs(args.begin(), args.end());
	vm->regRun(OpCode::PRINT_R, io_c_format(formatArgs, vm));
	return "";
}

PhsString StdLib::io_putf(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "putf", true);
	if (!args[0].isString())
		PHS_ERROR("putf() expects a string as its first argument (format)");
	std::vector<Value> formatArgs(args.begin(), args.end());
	PhsString        input = io_c_format(formatArgs, vm);
	vm->regRun(OpCode::PRINT_R, input.str() + "\n");
	return "";
}

#ifndef SANDBOXED
Value StdLib::io_gets(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "gets");
	return vm->regRun(OpCode::READLINE_R, REGISTER1);
}

Value StdLib::io_get_input(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "get_input");
	std::string content((std::istreambuf_iterator<char>(std::cin)), std::istreambuf_iterator<char>());
	return content;
}
#endif

PhsString StdLib::io_puts_error(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "puts_error", true);
	PhsString input = args[0].toString();
	vm->regRun(OpCode::PRINTERROR_R, input.str() + "\n");
	return "";
}

PhsString StdLib::io_print_error(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "puts_error", true);
	PhsString input = args[0].toString();
	vm->regRun(OpCode::PRINTERROR_R, input.str());
	return "";
}

PhsString StdLib::io_putf_error(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "putf_error", true);
	if (!args[0].isString())
		PHS_ERROR("putf_error() expects a string as its first argument (format)");
	std::vector<Value> formatArgs(args.begin(), args.end());
	PhsString input = io_c_format(formatArgs, vm);
	vm->regRun(OpCode::PRINTERROR_R, input.str() + "\n");
	return "";
}
} // namespace Phasor
