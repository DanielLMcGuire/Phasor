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

    if (!args[0].isStruct()) 
        PHS_ERROR("has() expects an struct as its first argument");

    auto object = args[0].asStruct();
    const Value& elementName = args[1];
    if (!elementName.isString())
        PHS_ERROR("has() expects a string as its second argument");

    return {object->fields.contains(elementName.string())};
}

Value StdLib::object_find(const Value::ArrayInstance &args, VM *)
{
    checkArgCount(args, 2, "struct_find", true);

    if (args.size() > 3)
        PHS_ERROR("struct_find() takes at most 3 arguments (array, elementName, value)");

    const Value &arrayToSearch = args[0];
    if (!arrayToSearch.isArray())
        PHS_ERROR("struct_find() expects an array as its first argument");

    const Value &elementNameArg = args[1];
    if (!elementNameArg.isString())
        PHS_ERROR("struct_find() expects a string as its second argument");

    const PhsString elementName = elementNameArg.string();
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
    if (!arrayToSearch.isArray())
        PHS_ERROR("filter() expects an array as its first argument");

    const Value &conditionsArg = args[1];
    if (!conditionsArg.isArray())
        PHS_ERROR("filter() expects an array of conditions as its second argument");

    struct Condition
    {
        PhsString name;
        Value value;
    };

    auto conditionsArr = conditionsArg.asArray();
    std::vector<Condition> conditions;
    conditions.reserve(conditionsArr->size());

    for (const Value &cond : *conditionsArr)
    {
        if (!cond.isStruct())
            PHS_ERROR("filter() conditions must be structs with elementName and value fields");

        if (!cond.hasField(PhsString("elementName")))
            PHS_ERROR("filter() condition is missing an 'elementName' field");

        Value nameVal = cond.getField(PhsString("elementName"));
        if (!nameVal.isString())
            PHS_ERROR("filter() condition 'elementName' must be a string");

        Value valueVal = cond.hasField(PhsString("value")) ? cond.getField(PhsString("value")) : Value();
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