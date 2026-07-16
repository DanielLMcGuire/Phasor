#include "StdLib.hpp"
#include <version.h>
#include <phsint.hpp>
#include <../ISA/map.hpp>
#include <../Codegen/PhasorStruct/PhasorStruct.hpp>
#include <../Codegen/Bytecode/BytecodeDeserializer.hpp>
#include <../Codegen/Bytecode/BytecodeSerializer.hpp>

#if defined(_WIN32)
  #include <windows.h>
  #include <psapi.h>
#elif defined(__linux__)
  #include <malloc.h>
  #include <sys/resource.h>
#elif defined(__APPLE__)
  #include <sys/resource.h>
#endif

namespace Phasor
{

void StdLib::registerMetaFunctions(VM *vm)
{
#ifndef SANDBOXED
	vm->registerNativeFunction("phs_op", StdLib::meta_operation);
	vm->registerNativeFunction("phs_stack_run", StdLib::meta_stack_run);
	// vm->registerNativeFunction("phs__phs_push", StdLib::meta_push);
	// vm->registerNativeFunction("phs__phs_pop", StdLib::meta_pop);
#endif
	vm->registerNativeFunction("phs_version", StdLib::meta_get_version);
	// vm->registerNativeFunction("phs__phs_alloc_info", StdLib::meta_get_alloc_info);
	// vm->registerNativeFunction("phs__get_self", StdLib::meta_get_self);
    // vm->registerNativeFunction("phs__run_program", StdLib::meta_run_program);
    // vm->registerNativeFunction("phs__run_program_function", StdLib::meta_run_program_function);
	vm->registerNativeFunction("get_registers", StdLib::meta_get_registers);
	// vm->registerNativeFunction("phs__load_bytecode", StdLib::meta_load_bytecode_from_file);
	// vm->registerNativeFunction("phs__save_bytecode", StdLib::meta_save_bytecode_to_file);
}

#ifndef SANDBOXED
i64 StdLib::meta_operation(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "phs_op");
	if (args.size() > 4)
		throw std::runtime_error("Function 'phs_op' expects at most 4 arguments, but got " +
		                         std::to_string(args.size()));
	if (!args[0].isInt() && !args[0].isString())
		throw std::runtime_error("Function 'phs_op' expects an OpCode (int/string) as the first argument");

	Phasor::OpCode opcode = args[0].isString() ? stringToOpCode(args[0].string()) : static_cast<OpCode>(args[0].asInt());

	auto ret = vm->operation(opcode,
	                         args.size() > 1 ? static_cast<int>(args[1].asInt()) : 0,
	                         args.size() > 2 ? static_cast<int>(args[2].asInt()) : 0,
	                         args.size() > 3 ? static_cast<int>(args[3].asInt()) : 0);
	return ret.asInt();
}

Value StdLib::meta_stack_run(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "phs_stack_run");
	if (!args[0].isInt() && !args[0].isString())
		throw std::runtime_error("Function 'phs_stack_run' expects an OpCode (int/string) as the first argument");
	Phasor::OpCode opcode = args[0].isString() ? stringToOpCode(args[0].string()) : static_cast<Phasor::OpCode>(args[0].asInt());

	for (size_t i = args.size(); i-- > 1;)
		vm->push(args[i]);

	vm->operation(opcode);
	return vm->pop();
}
#endif

PhsString StdLib::meta_get_version(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "phs_version");
	return PHASOR_VERSION_STRING;
}

Value StdLib::meta_get_alloc_info(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "phs__phs_alloc_info");

    // struct alloc_info {
    //     heap_used: int,
    //     stack_limit: int,
    //     heap_used_kb: int,
    //     stack_limit_kb: int
    // }
	Value result = {{
		{"heap_used", phsnull},
		{"stack_limit", phsnull},
		{"heap_used_kb", phsnull},
		{"stack_limit_kb", phsnull},
	}};

#if defined(_WIN32)
	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
		result["heap_used"] = static_cast<i64>(pmc.WorkingSetSize);

	result["stack_limit"] = 1024 * 1024;
#elif defined(__linux__)
	struct mallinfo2 mi = mallinfo2();
	result["heap_used"] = static_cast<i64>(mi.uordblks);

	struct rlimit rl{};
	getrlimit(RLIMIT_STACK, &rl);
	result["stack_limit"] = static_cast<i64>(rl.rlim_cur);
#elif defined (__APPLE__)
	struct rusage ru{};
	getrusage(RUSAGE_SELF, &ru);
	result["heap_used"] = static_cast<i64>(ru.ru_maxrss);

	struct rlimit rl{};
	getrlimit(RLIMIT_STACK, &rl);
	result["stack_limit"] = static_cast<i64>(rl.rlim_cur);
#endif

	result["heap_used_kb"] = result["heap_used"].asInt() / 1024;
	result["stack_limit_kb"] = result["stack_limit"].asInt()  / 1024;
	return result;
}

Value StdLib::meta_get_registers(const std::vector<Value> &args, VM *vm) 
{
	checkArgCount(args, 0, "get_registers");
	size_t registers = vm->getRegisterCount();
	auto reg_array = Value::createArray();
	for (const auto& i : std::views::iota(0u, registers))
	{
		reg_array.asArray()->push_back(vm->getRegister(i));
	}
	return reg_array;
}

Value StdLib::meta_get_self(const std::vector<Value> &args, VM *vm)
{
    checkArgCount(args, 0, "phs__get_self");
    auto bc = vm->getBytecode();

    return bytecodeToValue(bc, vm);
}

Value StdLib::meta_load_bytecode_from_file(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "phs__load_bytecode");
	std::filesystem::path bcFile = args[0].stl_string();
	BytecodeDeserializer deserializer;
	if (!std::filesystem::exists(bcFile))
		throw std::runtime_error("Bytecode file \"" + bcFile.string() + "\" does not exist!");
	auto bc = deserializer.loadFromFile(bcFile);
	return bytecodeToValue(bc);
}

bool StdLib::meta_save_bytecode_to_file(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "phs__save_bytecode");
	std::filesystem::path outFile = args[1].stl_string();
	BytecodeSerializer serializer;
	auto bc = bytecodeFromValue(args[0]);
	return serializer.saveToFile(bc, outFile);
}

i64 StdLib::meta_run_program(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 1, "phs__run_program");

    const Value& program = args[0];
    if (!program.isStruct())
        throw std::runtime_error("run_program expects a Bytecode struct");

    Phasor::VM vm;
    Phasor::Bytecode bc = bytecodeFromValue(program);
    Phasor::StdLib::registerFunctions(vm);
    vm.run(bc);
    return static_cast<i64>(vm.getStatus());
}

Value StdLib::meta_run_program_function(const std::vector<Value> &args, VM *)
{
    // program: Bytecode, functionName: string, func_arguments: any[], cli_arguments: string[]
    checkArgCount(args, 4, "phs__run_program_function");

    Phasor::Value program = args[0];
    PhsString functionName = args[1].string();
    if (!args[2].isArray())
        throw std::runtime_error("run_program_function expects func_arguments to be an array");
    auto func_arguments = args[2].asArray();

    if (!args[3].isArray())
        throw std::runtime_error("run_program_function expects cli_arguments to be an array");
    auto cli_arguments = args[3].asArray();

    std::vector<std::string> arg_strings;
    arg_strings.reserve(cli_arguments->size());
    for (const auto &arg : *cli_arguments) {
        if (!arg.isString())
            throw std::runtime_error("run_program_function expects cli_arguments to contain only strings");
        arg_strings.push_back(arg.string());
    }

    std::vector<char *> argv_data;
    argv_data.reserve(arg_strings.size());
    for (auto &arg_str : arg_strings)
        argv_data.push_back(const_cast<char *>(arg_str.c_str()));

    Phasor::VM vm;
    Phasor::Bytecode bc = bytecodeFromValue(program);
    Phasor::StdLib::argc = static_cast<int>(arg_strings.size());
    Phasor::StdLib::argv = argv_data.data();
    Phasor::StdLib::registerFunctions(vm);

    for (size_t i = func_arguments->size(); i-- > 0;) {
        vm.push((*func_arguments)[i]);
    }
    vm.push(static_cast<i64>(arg_strings.size()));

    auto ret = vm.runFunction(functionName, bc, true);
    return ret;
}

Value StdLib::meta_push(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "phs_push");
	vm->push(args[0]);
	return phsnull;
}

Value StdLib::meta_pop(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "phs_pop");
	return vm->pop();
}

} // namespace Phasor
