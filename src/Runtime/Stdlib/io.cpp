#include "StdLib.hpp"

Value StdLib::registerIOFunctions(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "include_stdio");
	vm->registerNativeFunction("c_fmt", StdLib::io_c_format);
	vm->registerNativeFunction("prints", StdLib::io_prints);
	vm->registerNativeFunction("printf", StdLib::io_printf);
	vm->registerNativeFunction("puts", StdLib::io_puts);
	vm->registerNativeFunction("putf", StdLib::io_putf);
	vm->registerNativeFunction("gets", StdLib::io_gets);
	vm->registerNativeFunction("puts_error", StdLib::io_puts_error);
	vm->registerNativeFunction("putf_error", StdLib::io_putf_error);
	return true;
}

Value StdLib::io_c_format(const std::vector<Value> &args, VM *vm) {
    if (args.empty()) {
        return Value("");  // Return empty string if no arguments
    }

    const std::string &fmt = args[0].asString();
    std::string out;
    size_t argIndex = 1;  // Start from 1 because args[0] is the format string

    for (size_t i = 0; i < fmt.size(); ++i) {
        if (fmt[i] == '%' && i + 1 < fmt.size()) {
            char spec = fmt[++i];  // Move to the format specifier
            if (argIndex < args.size()) {
                const Value &v = args[argIndex++];
                switch (spec) {
                    case 's': 
                        out += v.asString();
                        break;
                    case 'd': 
                        out += std::to_string(v.asInt());
                        break;
                    case 'f': 
                        out += std::to_string(v.asFloat());
                        break;
                    case '%': 
                        out += '%'; 
                        break;
                    default:
                        // Unknown specifier, include it literally
                        out += '%';
                        out += spec;
                        break;
                }
                continue;
            }
        }
        out += fmt[i];
    }
    return Value(out);
}

Value StdLib::io_prints(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "prints");
	vm->setRegister(VM::Register::r1, args[0]);       // Load string into r1
	vm->operation(OpCode::PRINT_R, VM::Register::r1); // Print register r1
	return Value("");
}

Value StdLib::io_printf(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "printf", true);
	std::vector<Value> formatArgs(args.begin(), args.end());
	vm->setRegister(VM::Register::r1, io_c_format(formatArgs, vm));
	vm->operation(OpCode::PRINT_R, VM::Register::r1);
	return Value("");
}

Value StdLib::io_puts(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "puts", true);
	std::string input = args[0].toString();
	vm->setRegister(VM::Register::r1, input + "\n");  // Load string into r1
	vm->operation(OpCode::PRINT_R, VM::Register::r1); // Print register r1
	return Value("");
}

Value StdLib::io_putf(const std::vector<Value> &args, VM *vm)
{
    checkArgCount(args, 1, "putf", true);
    std::vector<Value> formatArgs(args.begin(), args.end());
    std::string input = io_c_format(formatArgs, vm).toString();
    vm->setRegister(VM::Register::r1, input + "\n");  // Load string into r1
    vm->operation(OpCode::PRINT_R, VM::Register::r1); // Print register r1
    return Value("");
}

Value StdLib::io_gets(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "gets");
	vm->operation(OpCode::READLINE_R, VM::Register::r1);
	std::string ret = vm->getRegister(VM::Register::r1).toString();
	return ret;
}

Value StdLib::io_puts_error(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "puts_error", true);
	std::string input = args[0].toString();
	vm->setRegister(VM::Register::r1, input + "\n");              // Load string into r1
	return vm->operation(OpCode::PRINTERROR_R, VM::Register::r1); // Print register r1
}

Value StdLib::io_putf_error(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "putf_error", true);
	std::vector<Value> formatArgs(args.begin(), args.end());
	std::string input = io_c_format(formatArgs, vm).toString();
	vm->setRegister(VM::Register::r1, input + "\n");
	return vm->operation(OpCode::PRINTERROR_R, VM::Register::r1);
}