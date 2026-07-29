#include "StdLib.hpp"
#include <version.h>
#include <phsint.hpp>
#include "../../ISA/map.hpp"
#include "../../Codegen/PhasorStruct/PhasorStruct.hpp"
#include "../../Codegen/Bytecode/BytecodeDeserializer.hpp"
#include "../../Codegen/Bytecode/BytecodeSerializer.hpp"

#if defined(_WIN32)
    #include <windows.h>
    #include <psapi.h>
#elif defined(__linux__)
    #include <unistd.h>
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
	vm->registerNativeFunction("phs__phs_push", StdLib::meta_push);
	vm->registerNativeFunction("phs__phs_pop", StdLib::meta_pop);
#endif
	vm->registerNativeFunction("phs_version", StdLib::meta_get_version);
	vm->registerNativeFunction("phs__phs_alloc_info", StdLib::meta_get_alloc_info);
	vm->registerNativeFunction("phs__get_self", StdLib::meta_get_self);
    vm->registerNativeFunction("phs__run_program", StdLib::meta_run_program);
    vm->registerNativeFunction("phs__run_program_function", StdLib::meta_run_program_function);
	vm->registerNativeFunction("get_registers", StdLib::meta_get_registers);
	vm->registerNativeFunction("phs__load_bytecode", StdLib::meta_load_bytecode_from_file);
	vm->registerNativeFunction("phs__save_bytecode", StdLib::meta_save_bytecode_to_file);
}

#ifndef SANDBOXED
i64 StdLib::meta_operation(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "phs_op", true);
	if (args.size() > 4)
		PHS_ERROR("Function 'phs_op' expects at most 4 arguments, but got " +
		                         std::to_string(args.size()));
	if (!args[0].isInt() && !args[0].isString())
		PHS_ERROR("Function 'phs_op' expects an OpCode (int/string) as the first argument");

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
		PHS_ERROR("Function 'phs_stack_run' expects an OpCode (int/string) as the first argument");
	Phasor::OpCode opcode = args[0].isString() ? stringToOpCode(args[0].string()) : static_cast<Phasor::OpCode>(args[0].asInt());

	for (size_t i = args.size(); i-- > 1;) 
    {
		vm->push(args[i]);
    }

	vm->operation(opcode);
	return vm->pop();
}
#endif

PhsString StdLib::meta_get_version(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "phs_version");
	return PHASOR_VERSION_STRING;
}

i64 getPhysicalHeapUsage()
{
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS pmc{};
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        return static_cast<i64>(pmc.WorkingSetSize); // bytes
    }

#elif defined(__linux__)
    FILE* f = fopen("/proc/self/statm", "r");
    if (f)
    {
        long rssPages = 0;
        if (fscanf(f, "%*s %ld", &rssPages) == 1)
        {
            fclose(f);
            long pageSize = sysconf(_SC_PAGESIZE);
            return static_cast<i64>(rssPages) * static_cast<i64>(pageSize); // bytes
        }
        fclose(f);
    }

#elif defined(__APPLE__)
    mach_task_basic_info info{};
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(),
                  MACH_TASK_BASIC_INFO,
                  reinterpret_cast<task_info_t>(&info),
                  &count) == KERN_SUCCESS)
    {
        return static_cast<i64>(info.resident_size); // bytes
    }
#endif

    return 0;
}

Value StdLib::meta_get_alloc_info(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "phs__phs_alloc_info");

	return {
		std::initializer_list<std::pair<PhsString, Value>>{
            {"physical_heap_bytes_used", getPhysicalHeapUsage()}
    }};
}

Value StdLib::meta_get_registers(const std::vector<Value> &args, VM *vm) 
{
	checkArgCount(args, 0, "get_registers");
	size_t registers = vm->getRegisterCount();
	auto reg_array = Value::createArray();
	for (const auto& i : std::views::iota(0U, registers))
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
	if (!args[0].isString())
		PHS_ERROR("phs__load_bytecode() expects a string as its argument (file path)");
	std::filesystem::path bcFile = args[0].stl_string();
	BytecodeDeserializer deserializer;
	if (!std::filesystem::exists(bcFile))
		PHS_ERROR("Bytecode file \"" + bcFile.string() + "\" does not exist!");
	auto bc = deserializer.loadFromFile(bcFile);
	return bytecodeToValue(bc);
}

bool StdLib::meta_save_bytecode_to_file(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "phs__save_bytecode");
	if (!args[0].isStruct())
		PHS_ERROR("phs__save_bytecode() expects a Bytecode struct as its first argument");
	if (!args[1].isString())
		PHS_ERROR("phs__save_bytecode() expects a string as its second argument (file path)");
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
        PHS_ERROR("run_program expects a Bytecode struct");

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

    if (!args[0].isStruct())
        PHS_ERROR("run_program_function expects program to be a Bytecode struct");
    if (!args[1].isString())
        PHS_ERROR("run_program_function expects functionName to be a string");

    const Phasor::Value& program = args[0];
    PhsString functionName = args[1].string();
    if (!args[2].isArray())
        PHS_ERROR("run_program_function expects func_arguments to be an array");
    auto func_arguments = args[2].asArray();

    if (!args[3].isArray())
        PHS_ERROR("run_program_function expects cli_arguments to be an array");
    auto cli_arguments = args[3].asArray();

    std::vector<std::string> arg_strings;
    arg_strings.reserve(cli_arguments->size());
    for (const auto &arg : *cli_arguments)
    {
        if (!arg.isString())
            PHS_ERROR("run_program_function expects cli_arguments to contain only strings");
        arg_strings.push_back(arg.string());
    }

    std::vector<char *> argv_data;
    argv_data.reserve(arg_strings.size());
    for (auto &arg_str : arg_strings) 
    {
        argv_data.push_back(const_cast<char *>(arg_str.c_str()));
    }

    Phasor::VM vm;
    Phasor::Bytecode bc = bytecodeFromValue(program);
    Phasor::StdLib::argc = static_cast<int>(arg_strings.size());
    Phasor::StdLib::argv = argv_data.data();
    Phasor::StdLib::registerFunctions(vm);

    for (size_t i = func_arguments->size(); i-- > 0;)
    {
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
