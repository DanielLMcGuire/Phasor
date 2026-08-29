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

#include "StdLib.hpp"
#include <version.h>
#include <phsint.hpp>
#include <PhasorRT.h>
#include "../../ISA/map.hpp"
#include "../../Codegen/PhasorStruct/PhasorStruct.hpp"
#include "../../Language/Phasor/Lexer/Lexer.hpp"
#include "../../Language/Phasor/Parser/Parser.hpp"
#include "../../Language/Phasor/Parser/PlatformDefines.hpp"
#include "../../Codegen/CodeGen.hpp"
#include "../../Codegen/Bytecode/BytecodeSerializer.hpp"
#include "../../Codegen/Bytecode/BytecodeDeserializer.hpp"
#include <nativeerror.h>
#include <filesystem>
#include <sstream>

#if defined(_WIN32)
    #include <windows.h>
    #include <psapi.h>
#elif defined(__linux__)
    #include <unistd.h>
    #include <malloc.h>
    #include <sys/resource.h>
#elif defined(__APPLE__)
    #include <sys/resource.h>
#endif

namespace Phasor
{

namespace {
 
std::vector<std::filesystem::path> resolveIncludePathsFromValues(const Phasor::string &modulePath,
                                                                   const std::vector<Phasor::string> &includePaths)
{
    std::vector<std::filesystem::path> finalPaths;
 
#ifdef PHASOR_DEFAULT_FIRST_PATH
    finalPaths.emplace_back(PHASOR_DEFAULT_FIRST_PATH);
#endif
 
    Phasor::string envIncludeDirs;
    if (dupenv_ret ret = dupenv(envIncludeDirs, "PHASOR_INCLUDE_PATH"); ret == dupenv_ret::Success)
    {
        std::stringstream ss(envIncludeDirs.c_str());
        std::string item;
        while (std::getline(ss, item, ';'))
        {
            if (!item.empty())
            {
                finalPaths.emplace_back(item);
            }
        }
    }
 
    finalPaths.push_back(std::filesystem::current_path());
 
    if (!modulePath.empty())
    {
        finalPaths.emplace_back(modulePath.c_str());
    }
 
    for (const auto &path : includePaths)
    {
        if (!path.empty())
        {
            finalPaths.emplace_back(path.c_str());
        }
    }
 
    return finalPaths;
}
 
Defines resolveDefinesFromValues(const std::vector<Phasor::string> &defines)
{
    Defines finalDefines;
    addDefaultDefines(finalDefines, false);
 
    for (const auto &raw : defines)
    {
        if (raw.empty())
            continue;
 
        std::string item(raw.c_str());
        const size_t eq = item.find('=');
        if (eq == std::string::npos)
        {
            finalDefines[item] = DefineValue(DefineValueKind::Number, "1");
        }
        else
        {
            finalDefines[item.substr(0, eq)] = parseCliDefineValue(item.substr(eq + 1));
        }
    }
 
    return finalDefines;
}
 
} // namespace

void StdLib::registerMetaFunctions(VM *vm)
{
#ifndef SANDBOXED
	vm->registerNativeFunction("phs_op", StdLib::meta_operation);
	vm->registerNativeFunction("phs__stack_run", StdLib::meta_stack_run);
	vm->registerNativeFunction("phs__phs_push", StdLib::meta_push);
	vm->registerNativeFunction("phs__phs_pop", StdLib::meta_pop);

    vm->registerNativeFunction("phs__new_vm", StdLib::meta_new_state);
    vm->registerNativeFunction("phs__free_vm", StdLib::meta_free_state);
    vm->registerNativeFunction("phs__compile", StdLib::meta_compile_phs);
    vm->registerNativeFunction("phs__eval", StdLib::meta_eval_phs);
    vm->registerNativeFunction("phs__exec", StdLib::meta_exec_phsb);
#endif
	vm->registerNativeFunction("phs_version", StdLib::meta_get_version);
	vm->registerNativeFunction("phs__get_self", StdLib::meta_get_self);
	vm->registerNativeFunction("get_registers", StdLib::meta_get_registers);
	vm->registerNativeFunction("phs__load_bytecode", StdLib::meta_load_bytecode_from_file);
	vm->registerNativeFunction("phs__save_bytecode", StdLib::meta_save_bytecode_to_file);
}

#ifndef SANDBOXED
i64 StdLib::meta_operation(const Value::ArrayInstance &args, VM *vm)
{
	checkArgCount(args, 1, "phs_op", true);
	if (args.size() > 4)
		PHS_ERROR("Function 'phs_op' expects at most 4 arguments, but got " +
		                         std::to_string(args.size()));
	if (!args[0].isInt() && !args[0].isString())
		PHS_ERROR("Function 'phs_op' expects an OpCode (int/string) as the first argument");

	Phasor::OpCode opcode = args[0].isString() ? stringToOpCode(args[0].string()) : static_cast<OpCode>(args[0].asInt());

	auto ret = vm->operation(opcode,
	                         args.size() > 1 ? static_cast<int>(args[1].asInt()) : 0,
	                         args.size() > 2 ? static_cast<int>(args[2].asInt()) : 0,
	                         args.size() > 3 ? static_cast<int>(args[3].asInt()) : 0);
	return ret.asInt();
}

Value StdLib::meta_stack_run(const Value::ArrayInstance &args, VM *vm)
{
	checkArgCount(args, 1, "self_stack_run");
	if (!args[0].isInt() && !args[0].isString())
		PHS_ERROR("Function 'self_stack_run' expects an OpCode (int/string) as the first argument");
	Phasor::OpCode opcode = args[0].isString() ? stringToOpCode(args[0].string()) : static_cast<Phasor::OpCode>(args[0].asInt());

	for (size_t i = args.size(); i-- > 1;) 
    {
		vm->push(args[i]);
    }

	vm->operation(opcode);
	return vm->pop();
}
#endif

enum semver: Phasor::u8 {
    major, 
    minor,
    patch
};

Phasor::string StdLib::meta_get_version(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 0, "phs_version", true);
    if (args.size() > 1) PHS_ERROR("phs_version() expects at most 1 argument (version type)");
    if (args.size() == 1) requireInt(args[0], "phs_version", "version type");
    else return PHASOR_VERSION_STRING;

    semver ver = static_cast<semver>(args[0].asInt());
	switch (ver) {
        case major:
            return PHASOR_VERSION_MAJOR;
        case minor:
            return PHASOR_VERSION_MINOR;
        case patch:
            return PHASOR_VERSION_PATCH;
    }
    return PHASOR_VERSION_STRING;
}

Value StdLib::meta_get_registers(const Value::ArrayInstance &args, VM *vm) 
{
	checkArgCount(args, 0, "get_registers");
	size_t registers = vm->getRegisterCount();
	auto reg_array = Value::createArray();
	for (const auto& i : std::views::iota(0U, registers))
	{
		reg_array.asArray()->push_back(vm->getRegister(i));
	}
	return reg_array;
}

Value StdLib::meta_get_self(const Value::ArrayInstance &args, VM *vm)
{
    checkArgCount(args, 0, "phs__get_self");
    auto bc = vm->getBytecode();

    return bytecodeToValue(bc);
}

Value StdLib::meta_load_bytecode_from_file(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "phs__load_bytecode");
    requireString(args[0], "phs__load_bytecode", "file path");
	std::filesystem::path bcFile = args[0].stl_string();
	BytecodeDeserializer deserializer;
	if (!std::filesystem::exists(bcFile))
		PHS_ERROR("Bytecode file \"" + bcFile.string() + "\" does not exist!");
	auto bc = deserializer.loadFromFile(bcFile);
	return bytecodeToValue(bc);
}

bool StdLib::meta_save_bytecode_to_file(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "phs__save_bytecode");
    requireStruct(args[0], "phs__save_bytecode", "Bytecode");
    requireString(args[1], "phs__save_bytecode", "file path");
	std::filesystem::path outFile = args[1].stl_string();
	BytecodeSerializer serializer;
	auto bc = bytecodeFromValue(args[0]);
	return serializer.saveToFile(bc, outFile);
}

Value StdLib::meta_push(const Value::ArrayInstance &args, VM *vm)
{
	checkArgCount(args, 1, "phs_push");
	vm->push(args[0]);
	return phsnull;
}

Value StdLib::meta_pop(const Value::ArrayInstance &args, VM *vm)
{
	checkArgCount(args, 0, "phs_pop");
	return vm->pop();
}

i64 StdLib::meta_new_state(const Value::ArrayInstance &args, VM *) {
    checkArgCount(args, 0, "new_vm");
    auto *vm = new VM();
    return pointer_to_i64(vm);
}
 
bool StdLib::meta_free_state(const Value::ArrayInstance &args, VM *){
    checkArgCount(args, 2, "free_vm");
    requireInt(args[0], "free_vm", "state handle");
    auto *vm = static_cast<VM *>(i64_to_pointer(args[0].asInt()));
    if (vm == nullptr)
        return false;
    delete vm;
    return true;
}
 
i64 StdLib::meta_eval_phs(const Value::ArrayInstance &args, VM *){
    checkArgCount(args, 5, "phs_eval", true);
    if (args.size() > 6) PHS_ERROR("phs_eval() expects at least 5 arguments, at most 6 arguments.");
    if (!args[0].isInt() && !args[0].isNull())
        PHS_ERROR("phs_eval() expects a integer (state handle) or null as it's first argument.");
    requireString(args[1], "phs_eval", "source");
    requireString(args[2], "phs_eval", "module path");
    requireArray(args[3], "phs_eval", "include paths");
    requireArray(args[4], "phs_eval", "defines");
    if (args.size() == 6) requireBool(args[5], "phs_eval", "verbose");
 
    VM *state = nullptr;
    bool ownVM = false;
    if (args[0].isInt()) state = static_cast<VM *>(i64_to_pointer(args[0].asInt()));
    else {
        ownVM = true;
        state = new VM;
        if (!state) return -1;
    } 
    try {
        Phasor::string script = args[1].toString();
        Phasor::string modulePath = args[2].toString();
        std::vector<Phasor::string> includePaths = phasorStringArrayToVector(args[3]);
        std::vector<Phasor::string> defines = phasorStringArrayToVector(args[4]);
        bool verbose = args.size() == 6 ? args[5].asBool() : false;

        Lexer lexer(script.str());
        auto tokens = lexer.tokenize();
        Parser parser(tokens, modulePath.str());
    
        auto resolvedIncludes = resolveIncludePathsFromValues(modulePath, includePaths);
        if (!resolvedIncludes.empty())
        {
            parser.setIncludePaths(resolvedIncludes);
        }
        if (!modulePath.empty())
        {
            parser.setSourcePath(modulePath.str());
        }
        parser.setDefines(resolveDefinesFromValues(defines));
    
        auto program = parser.parse();
        if (verbose)
        {
            program->print();
        }
    
        CodeGenerator codegen;
        auto bytecode = codegen.generate(*program);
 
        i64 ret = static_cast<i64>(state->run(bytecode));
        if (ownVM) delete state;
        return ret;
    } catch(...) {
        if (ownVM) delete state;
        throw;
        return -1;
    }
    return -1;
}
 
i64 StdLib::meta_exec_phsb(const Value::ArrayInstance &args, VM *){
    checkArgCount(args, 2, "phs_exec");
    if (!args[0].isInt() && !args[0].isNull())
        PHS_ERROR("phs_exec() expects a integer (state handle) or null as it's first argument.");
    requireStruct(args[1], "phs_exec", "Bytecode");
 
    VM *state = nullptr;
    bool ownVM = false;
    if (args[0].isInt()) state = static_cast<VM *>(i64_to_pointer(args[0].asInt()));
    if (!state) {
        ownVM = true;
        state = new VM;
    }
 
    Bytecode bytecode = bytecodeFromValue(args[1]);
    int ret = static_cast<i64>(state->run(bytecode));
    if (ownVM) delete state;
    return ret;
}
 
Value StdLib::meta_compile_phs(const Value::ArrayInstance &args, VM *){
    checkArgCount(args, 5, "phs_compile");
    requireString(args[1], "phs_eval", "source");
    requireString(args[2], "phs_eval", "module path");
    requireArray(args[3], "phs_eval", "include paths");
    requireArray(args[4], "phs_eval", "defines");
 
    Phasor::string script = args[0].toString();
    Phasor::string modulePath = args[1].toString();
    std::vector<Phasor::string> includePaths = phasorStringArrayToVector(args[2]);
    std::vector<Phasor::string> defines = phasorStringArrayToVector(args[3]);
 
    Lexer lexer(script);
    Parser parser(lexer.tokenize(), modulePath.str());
 
    auto resolvedIncludes = resolveIncludePathsFromValues(modulePath, includePaths);
    if (!resolvedIncludes.empty())
    {
        parser.setIncludePaths(resolvedIncludes);
    }
    if (!modulePath.empty())
    {
        parser.setSourcePath(modulePath.str());
    }
    parser.setDefines(resolveDefinesFromValues(defines));
 
    auto ast = parser.parse();
    CodeGenerator codegen;
    Bytecode bytecode = codegen.generate(*ast);
 
    return bytecodeToValue(bytecode);
}


} // namespace Phasor
