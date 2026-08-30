// Copyright 2025-2026 Daniel McGuire
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

#include "StdLib.hpp"

namespace Phasor
{

void StdLib::registerObjectFunctions(VM *vm)
{
    vm->registerNativeFunction("has", object_has);
    vm->registerNativeFunction("struct_find", object_find);
    vm->registerNativeFunction("filter", object_filter);
}

Value StdLib::object_has(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 2, "has", false);

    requireStruct(args[0], "has", "struct");

    auto object = args[0].asStruct();
    const Value& elementName = args[1];
    requireString(elementName, "has", "string");

    return object->fields.contains(elementName.string());
}

Value StdLib::object_find(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 2, "struct_find", true);

    if (args.size() > 3)
        PHS_ERROR("struct_find() takes at most 3 arguments (array, elementName, value)");

    const Value &arrayToSearch = args[0];
    requireArray(arrayToSearch, "struct_find", "array");

    const Value &elementNameArg = args[1];
    requireString(elementNameArg, "struct_find", "string");

    const Phasor::string elementName = elementNameArg.string();
    const bool hasValueFilter = args.size() > 2;
    const Value *filterValue = hasValueFilter ? &args[2] : nullptr;

    auto arr = arrayToSearch.asArray();
    for (const Value &item : *arr)
    {
        if (!item.isStruct() || !item.hasField(elementName)) 
        {
            continue;
        }

        if (hasValueFilter && !(item.getField(elementName) == *filterValue)) 
        {
            continue;
        }

        return item;
    }

    return phsnull;
}

Value StdLib::object_filter(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 2, "filter", false);

    const Value &arrayToSearch = args[0];
    requireArray(arrayToSearch, "filter", "array");

    const Value &conditionsArg = args[1];
    requireArray(conditionsArg, "filter", "array of conditions");

    struct Condition
    {
        Phasor::string name;
        Value value;
    };

    auto conditionsArr = conditionsArg.asArray();
    std::vector<Condition> conditions;
    conditions.reserve(conditionsArr->size());

    for (const Value &cond : *conditionsArr)
    {
        if (!cond.isStruct())
            PHS_ERROR("filter() conditions must be structs with elementName and value fields");

        if (!cond.hasField(Phasor::string("elementName")))
            PHS_ERROR("filter() condition is missing an 'elementName' field");

        Value nameVal = cond.getField(Phasor::string("elementName"));
        if (!nameVal.isString())
            PHS_ERROR("filter() condition 'elementName' must be a string");

        Value valueVal = cond.hasField(Phasor::string("value")) ? cond.getField(Phasor::string("value")) : phsnull;
        conditions.push_back({nameVal.string(), std::move(valueVal)});
    }

    auto arr = arrayToSearch.asArray();
    Value::ArrayInstance matches;

    for (const Value &item : *arr)
    {
        if (!item.isStruct())
        {
            continue;
        }

        bool matchesAll = true;
        for (const auto &cond : conditions)
        {
            if (!item.hasField(cond.name) || !(item.getField(cond.name) == cond.value))
            {
                matchesAll = false;
                break;
            }
        }

        if (matchesAll)
        {
            matches.push_back(item);
        }
    }

    return Value::createArray(std::move(matches));
}

}