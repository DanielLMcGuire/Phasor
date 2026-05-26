#include "StdLib.hpp"
#include <version.h>
#include <phsint.hpp>
#include <../ISA/map.hpp>

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
#endif
	vm->registerNativeFunction("phs_version", StdLib::meta_get_version);
	vm->registerNativeFunction("phs_alloc_info", StdLib::meta_get_alloc_info);
	vm->registerNativeFunction("get_elements", StdLib::meta_get_struct_elements);
	vm->registerNativeFunction("get_elements_values", StdLib::meta_get_struct_elements_values);
	vm->registerNativeFunction("get_self", StdLib::meta_get_self);
	vm->registerNativeFunction("get_registers", StdLib::meta_get_registers);
	vm->registerNativeFunction("get_type", StdLib::meta_get_type);
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
	checkArgCount(args, 0, "phs_alloc_info");

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

Value StdLib::meta_get_struct_elements(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 1, "get_elements");

    const auto &structVal = args[0];
    if (!structVal.isStruct())
    {
        throw std::runtime_error("meta_get_struct_elements: argument is not a struct");
    }

    const auto structPtr = structVal.asStruct();
    if (!structPtr)
    {
        throw std::runtime_error("meta_get_struct_elements: struct instance is null");
    }

    std::vector<Value> keys;
    keys.reserve(structPtr->fields.size());

    for (const auto &[key, _] : structPtr->fields)
    {
        keys.push_back(key);
    }

    return Value::createArray(std::move(keys));
}

Value StdLib::meta_get_struct_elements_values(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "get_elements_values");

	const auto &structVal = args[0];
	if (!structVal.isStruct())
	{
		throw std::runtime_error("get_elements_values: argument is not a struct");
	}

	const auto structPtr = structVal.asStruct();
	if (!structPtr)
	{
		throw std::runtime_error("get_elements_values: struct instance is null");
	}

	std::vector<Value> values;
	values.reserve(structPtr->fields.size());

	for (const auto &[key, value] : structPtr->fields)
	{
		values.push_back(value);
	}

	return Value::createArray(std::move(values));
}

Value StdLib::meta_get_self(const std::vector<Value> &args, VM *vm)
{
    checkArgCount(args, 0, "get_self");
    auto bc = vm->getBytecode();

    // struct StructData {
    //     name: string,
    //     firstConstIndex: i64,
    //     fieldCount: i64,
    //     fieldNames: string[]
    // }
    //
    // struct FunctionData {
    //     name: string,
    //     entry: i64,
    //     paramCount: i64
    // }
    //
    // struct ConstantData {
    //     type: string,
    //     value: any
    // }
    //
    // struct VariableData {
    //     name: string,
    //     type: string,
    //     value: any
    // }
    //
    // struct InstructionData {
    //     op: string,
    //     operand1: i64,
    //     operand2: i64,
    //     operand3: i64
    // }
    //
    // struct Bytecode {
    //     instructions: InstructionData[],
    //     constants: ConstantData[],
    //     variables: VariableData[],
    //     functions: FunctionData[],
    //     structs: StructData[]
    // }

    auto bytecode_struct = Value::createStruct("Bytecode");

    auto inst_arr = Value::createArray();
    auto& inst_vec = *inst_arr.asArray();
    for (const auto& inst : bc.instructions) {
        auto inst_val = Value::createStruct("InstructionData");
        inst_val["op"] = opCodeToString(inst.op);
        inst_val["operand1"] = static_cast<i64>(inst.operand1);
        inst_val["operand2"] = static_cast<i64>(inst.operand2);
        inst_val["operand3"] = static_cast<i64>(inst.operand3);
        inst_vec.push_back(inst_val);
    }
    bytecode_struct["instructions"] = inst_arr;

    auto const_arr = Value::createArray();
    auto& const_vec = *const_arr.asArray();
    for (const auto& val : bc.constants) {
        auto const_info = Value::createStruct("ConstantData");
        const_info["type"] = Phasor::Value::typeToString(val.getType());
        const_info["value"] = val;
        const_vec.push_back(const_info);
    }
    bytecode_struct["constants"] = const_arr;

    auto vars_array = Value::createArray();
    auto& vars_vec = *vars_array.asArray();
    for (const auto& [name, idx] : bc.variables) {
        auto var = vm->getVariable(idx);
        auto var_info = Value::createStruct("VariableData");
        var_info["name"] = name;
        var_info["type"] = Phasor::Value::typeToString(var.getType());
        var_info["value"] = var;
        vars_vec.push_back(var_info);
    }
    bytecode_struct["variables"] = vars_array;

    auto funcs_arr = Value::createArray();
    auto& func_vec = *funcs_arr.asArray();
    for (const auto& [name, entry] : bc.functionEntries) {
        auto func_info = Value::createStruct("FunctionData");
        func_info["name"] = name;
        func_info["entry"] = static_cast<i64>(entry);
        
        i64 param_count = 0;
        auto it = bc.functionParamCounts.find(name);
        if (it != bc.functionParamCounts.end()) {
            param_count = it->second;
        }
        func_info["paramCount"] = param_count;
        
        func_vec.push_back(func_info);
    }
    bytecode_struct["functions"] = funcs_arr;

    auto structs_arr = Value::createArray();
    auto& structs_vec = *structs_arr.asArray();
    for (const auto& sinfo : bc.structs) {
        auto s_val = Value::createStruct("StructData");
        s_val["name"] = sinfo.name;
        s_val["firstConstIndex"] = static_cast<i64>(sinfo.firstConstIndex);
        s_val["fieldCount"] = static_cast<i64>(sinfo.fieldCount);
        
        auto fields_arr = Value::createArray();
        auto& fields_vec = *fields_arr.asArray();
        for (const auto& fname : sinfo.fieldNames) {
            fields_vec.push_back(Value(fname));
        }
        s_val["fieldNames"] = fields_arr;
        
        structs_vec.push_back(s_val);
    }
    bytecode_struct["structs"] = structs_arr;

    return bytecode_struct;
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

Value StdLib::meta_get_type(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "get_type");
	auto type = args[0].getType();
	return Value::typeToString(type);
}

} // namespace Phasor
