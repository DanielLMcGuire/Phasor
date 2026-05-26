#include "StdLib.hpp"
#include <version.h>
#include <phsint.hpp>

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
}

#ifndef SANDBOXED
i64 StdLib::meta_operation(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "phs_op");
	if (args.size() > 4)
		throw std::runtime_error("Function 'phs_op' expects at most 4 arguments, but got " +
		                         std::to_string(args.size()));
	if (!args[0].isInt())
		throw std::runtime_error("Function 'phs_op' expects an OpCode (int) as the first argument");

	auto ret = vm->operation(static_cast<Phasor::OpCode>(args[0].asInt()),
	                         args.size() > 1 ? static_cast<int>(args[1].asInt()) : 0,
	                         args.size() > 2 ? static_cast<int>(args[2].asInt()) : 0,
	                         args.size() > 3 ? static_cast<int>(args[3].asInt()) : 0);
	return ret.asInt();
}

Value StdLib::meta_stack_run(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 1, "phs_stack_run");
	if (!args[0].isInt())
		throw std::runtime_error("Function 'phs_stack_run' expects an OpCode (int) as the first argument");
	auto opcode = static_cast<Phasor::OpCode>(args[0].asInt());

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

} // namespace Phasor
