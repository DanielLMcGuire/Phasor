#include <Value.hpp>

#include "StdLib.hpp"

namespace Phasor
{

void StdLib::registerArrayFunctions(VM *vm)
{
    vm->registerNativeFunction("arr_resize", array_resize);
    vm->registerNativeFunction("arr_length", array_length);
    vm->registerNativeFunction("arr_push", array_push);
    vm->registerNativeFunction("arr_pop", array_pop);
    vm->registerNativeFunction("arr_insert", array_insert);
}

Value StdLib::array_resize(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 2, "arr_resize");

    auto arr = std::const_pointer_cast<Value::ArrayInstance>(args[0].asArray());
    if (!arr)
        throw std::runtime_error("arr_resize called on non-array");

    i64 newSize = args[1].asInt();
    if (newSize < 0)
        throw std::runtime_error("arr_resize new size cannot be negative");

    arr->resize(static_cast<size_t>(newSize));
    return arr;
}

i64 StdLib::array_length(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 1, "arr_length");
    auto arr = args[0].asArray();
    if (!arr)
        throw std::runtime_error("arr_length called on non-array");

    return static_cast<i64>(arr->size());
}

Value StdLib::array_push(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 2, "arr_push");

    auto arr = std::const_pointer_cast<Value::ArrayInstance>(args[0].asArray());
    if (!arr)
        throw std::runtime_error("arr_push called on non-array");

    arr->push_back(args[1]);
    return arr;
}

Value StdLib::array_pop(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 1, "arr_pop");
    auto arr = std::const_pointer_cast<Value::ArrayInstance>(args[0].asArray());
    if (!arr)
        throw std::runtime_error("arr_pop called on non-array");

    if (arr->empty())
        throw std::runtime_error("arr_pop called on empty array");

    Value val = arr->back();
    arr->pop_back();
    return val;
}

Value StdLib::array_insert(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 3, "arr_insert");

    auto arr = std::const_pointer_cast<Value::ArrayInstance>(args[0].asArray());
    if (!arr)
        throw std::runtime_error("arr_insert called on non-array");

    i64 index = args[1].asInt();
    if (index < 0 || index > static_cast<i64>(arr->size()))
        throw std::runtime_error("arr_insert index out of bounds");

    arr->insert(arr->begin() + static_cast<size_t>(index), args[2]);
    return arr;
}

}