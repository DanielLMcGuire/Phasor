#include "ffi.hpp"

namespace Phasor
{

/**
 * @brief Converts a C-style FFI value to a C++ VM value.
 * @param c_value The C-style value from the plugin.
 * @return The equivalent C++ value for the VM.
 * @note This function is safe for strings, as it copies the C string into a
 *       new C++ std::string, taking ownership of the memory.
 */
static Phasor::Value from_c_value(const PhasorValue &c_value)
{
	switch (c_value.type)
	{
	case PHASOR_TYPE_NULL:
		return Phasor::Value();
	case PHASOR_TYPE_BOOL:
		return Phasor::Value(c_value.as.b);
	case PHASOR_TYPE_INT:
		return Phasor::Value(c_value.as.i);
	case PHASOR_TYPE_FLOAT:
		return Phasor::Value(c_value.as.f);
	case PHASOR_TYPE_STRING:
		if (c_value.as.s)
		{
			return Phasor::Value(c_value.as.s);
		}
		return Phasor::Value("");
	case PHASOR_TYPE_ARRAY: {
		std::vector<Phasor::Value> cpp_elements;
		if (c_value.as.a.elements && c_value.as.a.count > 0)
		{
			cpp_elements.reserve(c_value.as.a.count);
			for (size_t i = 0; i < c_value.as.a.count; ++i)
			{
				cpp_elements.push_back(from_c_value(c_value.as.a.elements[i]));
			}
		}
		return Phasor::Value::createArray(std::move(cpp_elements));
	}
	default:
		return Phasor::Value();
	}
}

/**
 * @brief Converts a C++ VM value to a C-style FFI value.
 * @param cpp_value The C++ value from the VM.
 * @param string_arena A vector of unique_ptrs to manage the lifetime of C strings.
 *                     Any strings converted will be allocated and their memory will be
 *                     managed by this arena.
 * @return The equivalent C-style value for the plugin.
 */
static PhasorValue to_c_value(const Phasor::Value &cpp_value, std::vector<std::unique_ptr<char[]>> &string_arena,
                              std::vector<std::unique_ptr<PhasorValue[]>> &array_arena)
{
	switch (cpp_value.getType())
	{
	case ValueType::Null:
		return phasor_make_null();
	case ValueType::Bool:
		return phasor_make_bool(cpp_value.asBool());
	case ValueType::Int:
		return phasor_make_int(cpp_value.asInt());
	case ValueType::Float:
		return phasor_make_float(cpp_value.asFloat());
	case ValueType::String: {
		const auto &str = cpp_value.asString();
		auto        c_str = std::make_unique<char[]>(str.length() + 1);
		std::copy(str.begin(), str.end(), c_str.get());
		c_str[str.length()] = '\0';
		PhasorValue val = phasor_make_string(c_str.get());
		string_arena.push_back(std::move(c_str));
		return val;
	}
	case ValueType::Array: {
		const auto &cpp_array = *cpp_value.asArray();
		size_t      count = cpp_array.size();
		if (count == 0)
		{
			return phasor_make_array(nullptr, 0);
		}

		auto c_array = std::make_unique<PhasorValue[]>(count);
		for (size_t i = 0; i < count; ++i)
		{
			c_array[i] = to_c_value(cpp_array[i], string_arena, array_arena);
		}

		PhasorValue val = phasor_make_array(c_array.get(), count);
		array_arena.push_back(std::move(c_array));
		return val;
	}
	default:
		return phasor_make_null();
	}
}

static Phasor::Value c_native_func_wrapper(PhasorNativeFunction c_func, Phasor::VM* vm, const std::vector<Phasor::Value>& args)
{
    std::vector<std::unique_ptr<char[]>> string_arena;
    std::vector<std::unique_ptr<PhasorValue[]>> array_arena;

    std::vector<PhasorValue> c_args;
    c_args.reserve(args.size());
    for (const auto& arg : args)
    {
        c_args.push_back(to_c_value(arg, string_arena, array_arena));
    }

    PhasorValue c_result = c_func(reinterpret_cast<PhasorVM*>(vm), (int)c_args.size(), c_args.data());

    return from_c_value(c_result);
}

void register_native_c_func(PhasorVM* vm, const char* name, PhasorNativeFunction func)
{
    Phasor::VM* cpp_vm = reinterpret_cast<Phasor::VM*>(vm);

    auto wrapper = [func](const std::vector<Phasor::Value>& args, Phasor::VM* vm_param) -> Phasor::Value
    {
        return c_native_func_wrapper(func, vm_param, args);
    };

    cpp_vm->registerNativeFunction(name, wrapper);
}

}