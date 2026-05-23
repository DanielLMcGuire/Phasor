#include "StdLib.hpp"
#include <vformat.hpp>

namespace Phasor
{

void StdLib::registerIOFunctions(VM *vm)
{
	vm->registerNativeFunction("c_fmt", StdLib::io_c_format);
	vm->registerNativeFunction("prints", StdLib::io_prints);
	vm->registerNativeFunction("printf", StdLib::io_printf);
	vm->registerNativeFunction("puts", StdLib::io_puts);
	vm->registerNativeFunction("putf", StdLib::io_putf);
	vm->registerNativeFunction("puts_error", StdLib::io_puts_error);
	vm->registerNativeFunction("putf_error", StdLib::io_putf_error);
#ifndef SANDBOXED
	vm->registerNativeFunction("clear", StdLib::io_clear);
	vm->registerNativeFunction("gets", StdLib::io_gets);
#endif
}

#ifndef SANDBOXED
Value StdLib::io_clear(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "clear");
	vm->regRun(OpCode::PRINT_R, "\033[2J\033[H");
	return Value();
}
#endif

std::string StdLib::io_c_format(const std::vector<Value> &args, VM *)
{
	if (args.empty())
	{
		return ""; // Return empty string if no arguments
	}

	const std::string &fmt = args[0].asString();

	// Make vector of format args
	std::vector<Value> formatArgs(args.begin() + 1, args.end());

	return vformat::str_format_v(fmt.c_str(), formatArgs);
	
}

std::string StdLib::io_prints(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "prints");
	vm->regRun(OpCode::PRINT_R, args[0]);
	return "";
}

std::string StdLib::io_printf(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "printf", true);
	std::vector<Value> formatArgs(args.begin(), args.end());
	vm->regRun(OpCode::PRINT_R, io_c_format(formatArgs, vm));
	return "";
}

std::string StdLib::io_puts(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "puts", true);
	std::string input = args[0].toString();
	vm->regRun(OpCode::PRINT_R, input + "\n");
	return "";
}

std::string StdLib::io_putf(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "putf", true);
	std::vector<Value> formatArgs(args.begin(), args.end());
	std::string        input = io_c_format(formatArgs, vm);
	vm->regRun(OpCode::PRINT_R, input + "\n");
	return "";
}

#ifndef SANDBOXED
Value StdLib::io_gets(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "gets");
	return vm->regRun(OpCode::READLINE_R, REGISTER1);
}
#endif

std::string StdLib::io_puts_error(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "puts_error", true);
	std::string input = args[0].toString();
	vm->regRun(OpCode::PRINTERROR_R, input + "\n");
	return "";
}

std::string StdLib::io_putf_error(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "putf_error", true);
	std::vector<Value> formatArgs(args.begin(), args.end());
	std::string        input = io_c_format(formatArgs, vm);
	vm->regRun(OpCode::PRINTERROR_R, input + "\n");
	return "";
}
} // namespace Phasor