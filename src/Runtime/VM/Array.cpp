#ifndef CMAKE_PCH
#include "VM.hpp"
#endif

namespace Phasor {

void VM::registerArrayFunctions()
{
    registerNativeFunction("__array_literal", &VM::native_array_literal);
    registerNativeFunction("__get_elem",      &VM::native_get_elem);
    registerNativeFunction("__set_elem",      &VM::native_set_elem);
}

Value VM::native_array_literal(const std::vector<Value>& args, VM* /*vm*/)
{
    return Value::createArray(std::vector<Value>(args.begin(), args.end()));
}

Value VM::native_get_elem(const std::vector<Value>& args, VM* /*vm*/)
{
    if (args.size() < 2)
        throw std::runtime_error("__get_elem requires container and key/index");

    // Handle Array indexing
    if (args[0].isArray())
    {
        auto arr = args[0].asArray();
        i64 idx = args[1].asInt();
        if (idx < 0 || idx >= static_cast<i64>(arr->size()))
            return phsnull;
        return (*arr)[static_cast<size_t>(idx)];
    }
    
    // Handle Struct field access
    // We treat the second argument as a key string for field lookup
    return args[0].getField(args[1].asString());
}

Value VM::native_set_elem(const std::vector<Value>& args, VM* /*vm*/)
{
    if (args.size() < 3)
        throw std::runtime_error("__set_elem requires container, key/index, and value");

    // Handle Array assignment
    if (args[0].isArray())
    {
        auto arr = std::const_pointer_cast<Value::ArrayInstance>(args[0].asArray());
        i64 idx = args[1].asInt();
        if (idx < 0 || idx >= static_cast<i64>(arr->size()))
            throw std::runtime_error("Index out of bounds");

        (*arr)[static_cast<size_t>(idx)] = args[2];
        return args[2];
    }

    // Handle Struct field assignment
    // 1. Create a mutable copy of the value from the const args list
    Value mutableObj = args[0]; 
    
    // 2. Call setField on the mutable copy
    mutableObj.setField(args[1].asString(), args[2]);
    
    // 3. Return the value assigned
    return args[2];
}

} // namespace Phasor