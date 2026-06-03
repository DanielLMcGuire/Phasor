#include "../../Runtime/VM/VM.hpp"
#include "../../Runtime/Stdlib/StdLib.hpp"
#include "../../ISA/map.hpp"

namespace Phasor 
{

inline Bytecode bytecodeFromValue(Value program) {
    Bytecode bc;

    auto readStringArray = [](const Value& v) -> std::vector<std::string> {
        std::vector<std::string> out;
        if (!v.isArray())
            return out;
        for (const auto& item : *v.asArray())
            out.push_back(item.asString());
        return out;
    };

    auto readIntArray = [](const Value& v) -> std::vector<int> {
        std::vector<int> out;
        if (!v.isArray())
            return out;
        for (const auto& item : *v.asArray())
            out.push_back(static_cast<int>(item.asInt()));
        return out;
    };

    auto readIntMatrix = [&](const Value& v) -> std::vector<std::vector<int>> {
        std::vector<std::vector<int>> out;
        if (!v.isArray())
            return out;
        for (const auto& row : *v.asArray())
            out.push_back(readIntArray(row));
        return out;
    };

    if (program.hasField("instructions") && program["instructions"].isArray()) {
        for (const auto& instVal : *program["instructions"].asArray()) {
            Instruction inst;
            inst.op       = stringToOpCode(instVal["op"].asString());
            inst.operand1 = static_cast<int>(instVal["operand1"].asInt());
            inst.operand2 = static_cast<int>(instVal["operand2"].asInt());
            inst.operand3 = static_cast<int>(instVal["operand3"].asInt());
            bc.instructions.push_back(inst);
        }
    }

    if (program.hasField("constants") && program["constants"].isArray()) {
        for (const auto& constVal : *program["constants"].asArray()) {
            if (constVal.hasField("value"))
                bc.constants.push_back(constVal["value"]);
            else
                bc.constants.push_back(phsnull);
        }
    }

    if (program.hasField("variables") && program["variables"].isArray()) {
        for (const auto& varVal : *program["variables"].asArray()) {
            const std::string name = varVal["name"].asString();
            const int index        = static_cast<int>(varVal["index"].asInt());
            bc.variables[name] = index;
        }
    }

    if (program.hasField("functions") && program["functions"].isArray()) {
        for (const auto& funcVal : *program["functions"].asArray()) {
            const std::string name  = funcVal["name"].asString();
            const int entry         = static_cast<int>(funcVal["entry"].asInt());

            bc.functionEntries[name] = entry;

            std::vector<std::string> paramTypes;
            if (funcVal.hasField("paramTypes"))
                paramTypes = readStringArray(funcVal["paramTypes"]);

            bc.functionParamCounts[name] = static_cast<int>(paramTypes.size());
            bc.functionParamTypeNames[name] = paramTypes;

            std::vector<std::vector<int>> paramDims;
            if (funcVal.hasField("paramArrayDims"))
                paramDims = readIntMatrix(funcVal["paramArrayDims"]);
            bc.functionParamArrayDims[name] = paramDims;

            if (funcVal.hasField("returnType"))
                bc.functionReturnTypeNames[name] = funcVal["returnType"].asString();
            else
                bc.functionReturnTypeNames[name] = "<unknown>";

            std::vector<int> returnDims;
            if (funcVal.hasField("returnArrayDims"))
                returnDims = readIntArray(funcVal["returnArrayDims"]);
            bc.functionReturnArrayDims[name] = returnDims;
        }
    }

    if (program.hasField("structs") && program["structs"].isArray()) {
        for (const auto& structVal : *program["structs"].asArray()) {
            StructInfo sinfo;
            sinfo.name = structVal["name"].asString();
            sinfo.firstConstIndex = static_cast<int>(structVal["firstConstIndex"].asInt());
            sinfo.fieldCount = static_cast<int>(structVal["fieldCount"].asInt());

            if (structVal.hasField("fieldNames"))
                sinfo.fieldNames = readStringArray(structVal["fieldNames"]);

            bc.structs.push_back(std::move(sinfo));
        }
    }

    bc.scopeVarLists.clear();

    auto readScopes = [&](const Value& scopesVal) {
        if (!scopesVal.isArray())
            return;

        for (const auto& scopeVal : *scopesVal.asArray()) {
            const int scopeIndex = static_cast<int>(scopeVal["scopeIndex"].asInt());
            if (scopeIndex >= (int)bc.scopeVarLists.size())
                bc.scopeVarLists.resize(scopeIndex + 1);

            if (!scopeVal.hasField("variables") || !scopeVal["variables"].isArray())
                continue;

            for (const auto& varVal : *scopeVal["variables"].asArray()) {
                const int varIndex = static_cast<int>(varVal["index"].asInt());
                const std::string varName = varVal["name"].asString();
                bc.scopeVarLists[scopeIndex].push_back({varIndex, varName});
            }
        }
    };

    if (program.hasField("globalScopes"))
        readScopes(program["globalScopes"]);

    if (program.hasField("functions") && program["functions"].isArray()) {
        for (const auto& funcVal : *program["functions"].asArray()) {
            if (funcVal.hasField("scopes"))
                readScopes(funcVal["scopes"]);
        }
    }
    return bc;
}

inline Value bytecodeToValue(Bytecode &bc, VM *vm) {
    std::vector<std::pair<int, std::string>> sortedFuncs;
    sortedFuncs.reserve(bc.functionEntries.size());
    for (const auto& [name, entry] : bc.functionEntries)
        sortedFuncs.push_back({entry, name});
    std::sort(sortedFuncs.begin(), sortedFuncs.end());

    std::unordered_map<int, std::string> scopeOwner;
    for (size_t fi = 0; fi < sortedFuncs.size(); ++fi) {
        int start        = sortedFuncs[fi].first;
        int end          = (fi + 1 < sortedFuncs.size())
                         ? sortedFuncs[fi + 1].first
                         : (int)bc.instructions.size();
        const auto& fname = sortedFuncs[fi].second;
        for (int i = start; i < end; ++i) {
            if (opCodeToString(bc.instructions[i].op) == "EXIT_SCOPE")
                scopeOwner[bc.instructions[i].operand1] = fname;
        }
    }

    auto makeScopeVal = [&](int si) {
        auto scope_val = Value::createStruct("ScopeData");
        scope_val["scopeIndex"] = static_cast<i64>(si);

        auto vars_arr = Value::createArray();
        auto& vars_vec = *vars_arr.asArray();

        for (const auto& [varIdx, varName] : bc.scopeVarLists[si]) {
            auto var_val = Value::createStruct("ScopeVariableData");
            var_val["index"] = static_cast<i64>(varIdx);
            var_val["name"]  = varName;
            vars_vec.push_back(var_val);
        }

        scope_val["variables"] = vars_arr;
        return scope_val;
    };

    auto bytecode_struct = Value::createStruct("Bytecode");

    auto inst_arr = Value::createArray();
    auto& inst_vec = *inst_arr.asArray();
    for (const auto& inst : bc.instructions) {
        auto inst_val = Value::createStruct("InstructionData");
        inst_val["op"]       = opCodeToString(inst.op);
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
        const_info["type"]  = Value::typeToString(val.getType());
        const_info["value"] = val;
        const_vec.push_back(const_info);
    }
    bytecode_struct["constants"] = const_arr;

    auto vars_array = Value::createArray();
    auto& vars_vec = *vars_array.asArray();
    for (const auto& [name, idx] : bc.variables) {
        auto var      = vm->getVariable(idx);
        auto var_info = Value::createStruct("VariableData");
        var_info["name"]  = name;
        var_info["index"] = static_cast<i64>(idx);
        var_info["type"]  = Value::typeToString(var.getType());
        var_info["value"] = var;
        vars_vec.push_back(var_info);
    }
    bytecode_struct["variables"] = vars_array;

    auto funcs_arr = Value::createArray();
    auto& func_vec = *funcs_arr.asArray();
    for (const auto& [name, entry] : bc.functionEntries) {
        auto func_info = Value::createStruct("FunctionData");
        func_info["name"]  = name;
        func_info["entry"] = static_cast<i64>(entry);

        i64 param_count = 0;
        if (auto it = bc.functionParamCounts.find(name); it != bc.functionParamCounts.end())
            param_count = it->second;

        auto param_types_arr = Value::createArray();
        auto param_dims_arr  = Value::createArray();
        auto& ptv = *param_types_arr.asArray();
        auto& pdv = *param_dims_arr.asArray();

        auto param_names_it = bc.functionParamTypeNames.find(name);
        auto param_dims_it   = bc.functionParamArrayDims.find(name);
        for (int i = 0; i < param_count; ++i) {
            ptv.push_back(
                (param_names_it != bc.functionParamTypeNames.end() && i < (int)param_names_it->second.size())
                ? Value(param_names_it->second[i])
                : Value("<unknown>")
            );

            auto dim_arr = Value::createArray();
            auto& dv = *dim_arr.asArray();
            if (param_dims_it != bc.functionParamArrayDims.end() && i < (int)param_dims_it->second.size())
                for (int d : param_dims_it->second[i])
                    dv.push_back(static_cast<i64>(d));
            pdv.push_back(dim_arr);
        }
        func_info["paramTypes"]    = param_types_arr;
        func_info["paramArrayDims"] = param_dims_arr;

        auto ret_name_it = bc.functionReturnTypeNames.find(name);
        func_info["returnType"] = (ret_name_it != bc.functionReturnTypeNames.end())
                                ? Value(ret_name_it->second)
                                : Value("<unknown>");

        auto ret_dims_arr = Value::createArray();
        auto& rdv = *ret_dims_arr.asArray();
        if (auto it = bc.functionReturnArrayDims.find(name); it != bc.functionReturnArrayDims.end())
            for (int d : it->second)
                rdv.push_back(static_cast<i64>(d));
        func_info["returnArrayDims"] = ret_dims_arr;

        auto func_scopes_arr = Value::createArray();
        auto& fsv = *func_scopes_arr.asArray();
        for (int si = 0; si < (int)bc.scopeVarLists.size(); ++si)
            if (auto it = scopeOwner.find(si); it != scopeOwner.end() && it->second == name)
                fsv.push_back(makeScopeVal(si));
        func_info["scopes"] = func_scopes_arr;

        func_vec.push_back(func_info);
    }
    bytecode_struct["functions"] = funcs_arr;

    auto global_scopes_arr = Value::createArray();
    auto& gsv = *global_scopes_arr.asArray();
    for (int si = 0; si < (int)bc.scopeVarLists.size(); ++si)
        if (scopeOwner.find(si) == scopeOwner.end())
            gsv.push_back(makeScopeVal(si));
    bytecode_struct["globalScopes"] = global_scopes_arr;

    auto structs_arr = Value::createArray();
    auto& structs_vec = *structs_arr.asArray();
    for (const auto& sinfo : bc.structs) {
        auto s_val = Value::createStruct("StructData");
        s_val["name"]            = sinfo.name;
        s_val["firstConstIndex"] = static_cast<i64>(sinfo.firstConstIndex);
        s_val["fieldCount"]      = static_cast<i64>(sinfo.fieldCount);

        auto fields_arr = Value::createArray();
        auto& fields_vec = *fields_arr.asArray();
        for (const auto& fname : sinfo.fieldNames)
            fields_vec.push_back(Value(fname));
        s_val["fieldNames"] = fields_arr;

        structs_vec.push_back(s_val);
    }
    bytecode_struct["structs"] = structs_arr;

    return bytecode_struct;
}

}