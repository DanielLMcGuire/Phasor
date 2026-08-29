// Copyright 2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://phasor.pages.dev/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
    vm->registerNativeFunction("arr_join", array_join);
    vm->registerNativeFunction("arr_find", array_find);
}

Value StdLib::array_resize(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 2, "arr_resize");

    requireArray(args[0], "arr_resize", "array");
    requireInt(args[1], "arr_resize", "integer");

    auto arr = const_cast<Value &>(args[0]).asArray();

    i64 newSize = args[1].asInt();

    if (newSize < 0)
        PHS_ERROR("arr_resize() new size cannot be negative");

    arr->resize(static_cast<size_t>(newSize));
    return arr;
}

i64 StdLib::array_length(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 1, "arr_length");
    requireArray(args[0], "arr_length", "array");
    auto arr = const_cast<Value &>(args[0]).asArray();


    return static_cast<i64>(arr->size());
}

Value StdLib::array_push(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 2, "arr_push");

    requireArray(args[0], "arr_push", "array");
    auto arr = const_cast<Value &>(args[0]).asArray();

    arr->push_back(args[1]);

    return arr;
}

Value StdLib::array_pop(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 1, "arr_pop");
    requireArray(args[0], "arr_pop", "array");
    auto arr = const_cast<Value &>(args[0]).asArray();


    if (arr->empty())
        PHS_ERROR("arr_pop() called on empty array");

    Value val = arr->back();
    arr->pop_back();
    return val;
}

Value StdLib::array_peek(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 1, "arr_peek");
    requireArray(args[0], "arr_peek", "array");
    auto arr = const_cast<Value &>(args[0]).asArray();


    if (arr->empty())
        PHS_ERROR("arr_peek() called on empty array");

    Value val = arr->back();
    return val;
}

Value StdLib::array_insert(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 3, "arr_insert");

    requireArray(args[0], "arr_insert", "array");
    requireInt(args[1], "arr_insert", "index");

    auto arr = const_cast<Value &>(args[0]).asArray();

    i64 index = args[1].asInt();

    if (index < 0 || std::cmp_greater(index ,arr->size()))
        PHS_ERROR("arr_insert() index out of bounds");

    arr->insert(arr->begin() + static_cast<size_t>(index), args[2]);
    return arr;
}

Value StdLib::array_join(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 2, "arr_join");

    requireArray(args[0], "arr_find", "array");
    requireString(args[1], "arr_find", "string");

    auto arr = args[0].asArray();

    if (!arr)
        return "";

    const Phasor::string separator = args[1].string();

    Phasor::string result;

    for (size_t i = 0; i < arr->size(); ++i)
    {
        if (i != 0)
            result += separator;

        result += (*arr)[i].toString();
    }

    return result;
}

Value StdLib::array_find(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "arr_find");

    requireArray(args[0], "arr_find", "array");

	auto arr = args[0].asArray();
	const Value &needle = args[1];

	for (const Value &elem : *arr)
	{
		if (elem == needle)
			return true;
	}

	return false;
}

}
