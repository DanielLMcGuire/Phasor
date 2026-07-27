#include <Value.hpp>
#include <utility>

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

    if (!args[0].isArray())
        PHS_ERROR("arr_resize() expects an array as its first argument");

    if (!args[1].isInt())
        PHS_ERROR("arr_resize() expects an integer as its second argument");

    auto arr = const_cast<Value &>(args[0]).asArray();

    i64 newSize = args[1].asInt();

    if (newSize < 0)
        PHS_ERROR("arr_resize() new size cannot be negative");

    arr->resize(static_cast<size_t>(newSize));
    return arr;
}

i64 StdLib::array_length(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 1, "arr_length");
    if (!args[0].isArray())
        PHS_ERROR("arr_length() expects an array as its first argument");
    auto arr = const_cast<Value &>(args[0]).asArray();


    return static_cast<i64>(arr->size());
}

Value StdLib::array_push(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 2, "arr_push");

    if (!args[0].isArray())
        PHS_ERROR("arr_push() expects an array as its first argument");
    auto arr = const_cast<Value &>(args[0]).asArray();

    arr->push_back(args[1]);

    return arr;
}

Value StdLib::array_pop(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 1, "arr_pop");
    if (!args[0].isArray())
        PHS_ERROR("arr_pop() expects an array as its first argument");
    auto arr = const_cast<Value &>(args[0]).asArray();


    if (arr->empty())
        PHS_ERROR("arr_pop() called on empty array");

    Value val = arr->back();
    arr->pop_back();
    return val;
}

Value StdLib::array_insert(const std::vector<Value> &args, VM *)
{
    checkArgCount(args, 3, "arr_insert");

    if (!args[0].isArray())
        PHS_ERROR("arr_insert() expects an array as its first argument");

    if (!args[1].isInt())
        PHS_ERROR("arr_insert() expects an integer as its second argument (index)");

    auto arr = const_cast<Value &>(args[0]).asArray();

    i64 index = args[1].asInt();

    if (index < 0 || std::cmp_greater(index ,arr->size()))
        PHS_ERROR("arr_insert() index out of bounds");

    arr->insert(arr->begin() + static_cast<size_t>(index), args[2]);
    return arr;
}

}
