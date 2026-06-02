#include "PhasorIR.hpp"
#include "../../ISA/map.hpp"
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <functional>
#include <map>
#include <filesystem>
#include <string>
#include <vector>
#include <phsint.hpp>

namespace Phasor
{

int PhasorIR::getOperandCount(OpCode op)
{
    switch (op)
    {
    // 0 operands
    case OpCode::POP:
    case OpCode::IADD:
    case OpCode::ISUBTRACT:
    case OpCode::IMULTIPLY:
    case OpCode::IDIVIDE:
    case OpCode::IMODULO:
    case OpCode::FLADD:
    case OpCode::FLSUBTRACT:
    case OpCode::FLMULTIPLY:
    case OpCode::FLDIVIDE:
    case OpCode::FLMODULO:
    case OpCode::SQRT:
    case OpCode::POW:
    case OpCode::LOG:
    case OpCode::EXP:
    case OpCode::SIN:
    case OpCode::COS:
    case OpCode::TAN:
    case OpCode::NEGATE:
    case OpCode::NOT:
    case OpCode::IAND:
    case OpCode::IOR:
    case OpCode::IEQUAL:
    case OpCode::INOT_EQUAL:
    case OpCode::ILESS_THAN:
    case OpCode::IGREATER_THAN:
    case OpCode::ILESS_EQUAL:
    case OpCode::IGREATER_EQUAL:
    case OpCode::FLEQUAL:
    case OpCode::FLNOT_EQUAL:
    case OpCode::FLLESS_THAN:
    case OpCode::FLGREATER_THAN:
    case OpCode::FLLESS_EQUAL:
    case OpCode::FLGREATER_EQUAL:
    case OpCode::PRINT:
    case OpCode::PRINTERROR:
    case OpCode::READLINE:
    case OpCode::HALT:
    case OpCode::RETURN:
    case OpCode::TRUE_P:
    case OpCode::FALSE_P:
    case OpCode::NULL_VAL:
    case OpCode::LEN:
    case OpCode::CHAR_AT:
    case OpCode::SUBSTR:
        return 0;

    // 1 operand
    case OpCode::PUSH_CONST:
    case OpCode::JUMP:
    case OpCode::JUMP_IF_FALSE:
    case OpCode::JUMP_IF_TRUE:
    case OpCode::JUMP_BACK:
    case OpCode::STORE_VAR:
    case OpCode::LOAD_VAR:
    case OpCode::IMPORT:
    case OpCode::CALL_NATIVE:
    case OpCode::CALL:
    case OpCode::SYSTEM:
    case OpCode::SYSTEM_OUT:
    case OpCode::SYSTEM_ERR:
    case OpCode::PUSH_R:
    case OpCode::POP_R:
    case OpCode::PRINT_R:
    case OpCode::PRINTERROR_R:
    case OpCode::READLINE_R:
    case OpCode::SYSTEM_R:
    case OpCode::SYSTEM_OUT_R:
    case OpCode::SYSTEM_ERR_R:
    case OpCode::NEW_STRUCT:
    case OpCode::GET_FIELD:
    case OpCode::SET_FIELD:
    case OpCode::NEW_STRUCT_INSTANCE_STATIC:
    case OpCode::EXIT_SCOPE:
        return 1;

    // 2 operands
    case OpCode::MOV:
    case OpCode::LOAD_CONST_R:
    case OpCode::LOAD_VAR_R:
    case OpCode::STORE_VAR_R:
    case OpCode::SQRT_R:
    case OpCode::LOG_R:
    case OpCode::EXP_R:
    case OpCode::SIN_R:
    case OpCode::COS_R:
    case OpCode::TAN_R:
    case OpCode::NEG_R:
    case OpCode::NOT_R:
    case OpCode::PUSH2_R:
    case OpCode::POP2_R:
    case OpCode::GET_FIELD_STATIC:
    case OpCode::SET_FIELD_STATIC:
        return 2;

    // 3 operands
    case OpCode::IADD_R:
    case OpCode::ISUB_R:
    case OpCode::IMUL_R:
    case OpCode::IDIV_R:
    case OpCode::IMOD_R:
    case OpCode::FLADD_R:
    case OpCode::FLSUB_R:
    case OpCode::FLMUL_R:
    case OpCode::FLDIV_R:
    case OpCode::FLMOD_R:
    case OpCode::POW_R:
    case OpCode::IAND_R:
    case OpCode::IOR_R:
    case OpCode::IEQ_R:
    case OpCode::INE_R:
    case OpCode::ILT_R:
    case OpCode::IGT_R:
    case OpCode::ILE_R:
    case OpCode::IGE_R:
    case OpCode::FLAND_R:
    case OpCode::FLOR_R:
    case OpCode::FLEQ_R:
    case OpCode::FLNE_R:
    case OpCode::FLLT_R:
    case OpCode::FLGT_R:
    case OpCode::FLLE_R:
    case OpCode::FLGE_R:
        return 3;

    default:
        return 0;
    }
}

PhasorIR::OperandType PhasorIR::getOperandType(OpCode op, int operandIndex)
{
    // Stack operations with special indices
    if (op == OpCode::PUSH_CONST && operandIndex == 0)
        return OperandType::CONSTANT_IDX;
    if (op == OpCode::STORE_VAR && operandIndex == 0)
        return OperandType::VARIABLE_IDX;
    if (op == OpCode::LOAD_VAR && operandIndex == 0)
        return OperandType::VARIABLE_IDX;
    if (op == OpCode::IMPORT && operandIndex == 0)
        return OperandType::CONSTANT_IDX;
    if (op == OpCode::CALL_NATIVE && operandIndex == 0)
        return OperandType::CONSTANT_IDX;
    if (op == OpCode::CALL && operandIndex == 0)
        return OperandType::FUNCTION_IDX;
    if (op == OpCode::SYSTEM && operandIndex == 0)
        return OperandType::CONSTANT_IDX;

    // Register operations with mixed types
    if (op == OpCode::LOAD_CONST_R)
    {
        if (operandIndex == 0) return OperandType::REGISTER;
        if (operandIndex == 1) return OperandType::CONSTANT_IDX;
    }
    if (op == OpCode::LOAD_VAR_R)
    {
        if (operandIndex == 0) return OperandType::REGISTER;
        if (operandIndex == 1) return OperandType::VARIABLE_IDX;
    }
    if (op == OpCode::STORE_VAR_R)
    {
        if (operandIndex == 0) return OperandType::REGISTER;
        if (operandIndex == 1) return OperandType::VARIABLE_IDX;
    }

    // JUMP instructions take an offset (INT)
    if (op == OpCode::JUMP || op == OpCode::JUMP_IF_FALSE ||
        op == OpCode::JUMP_IF_TRUE || op == OpCode::JUMP_BACK)
        return OperandType::INT;

    // Register ops use REGISTER for all operands
    if (static_cast<int>(op) >= static_cast<int>(OpCode::MOV))
        return OperandType::REGISTER;

    return OperandType::INT;
}

std::string PhasorIR::escapeString(const std::string &str)
{
    std::stringstream ss;
    for (unsigned char c : str)
    {
        switch (c)
        {
        case '\a':   ss << "\\a";   break;
        case '\b':   ss << "\\b";   break;
        case '\f':   ss << "\\f";   break;
        case '\n':   ss << "\\n";   break;
        case '\r':   ss << "\\r";   break;
        case '\t':   ss << "\\t";   break;
        case '\v':   ss << "\\v";   break;
        case '\x1b': ss << "\\e";   break;
        case '\\':   ss << "\\\\";  break;
        case '"':    ss << "\\\"";  break;
        default:
            if (c < 0x20 || c == 0x7F)
            {
                ss << "\\x"
                   << "0123456789abcdef"[c >> 4]
                   << "0123456789abcdef"[c & 0xF];
            }
            else
            {
                ss << static_cast<char>(c);
            }
            break;
        }
    }
    return ss.str();
}

std::string PhasorIR::unescapeString(const std::string &str)
{
    std::string result;
    size_t      len = str.length();

    auto hexVal = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };

    for (size_t i = 0; i < len; ++i)
    {
        if (str[i] != '\\' || i + 1 >= len)
        {
            result += str[i];
            continue;
        }

        char esc = str[++i];
        switch (esc)
        {
        case 'a':  result += '\a'; break;
        case 'b':  result += '\b'; break;
        case 'f':  result += '\f'; break;
        case 'n':  result += '\n'; break;
        case 'r':  result += '\r'; break;
        case 't':  result += '\t'; break;
        case 'v':  result += '\v'; break;
        case '\\': result += '\\'; break;
        case '\'': result += '\''; break;
        case '"':  result += '"';  break;
        case 'e':
        case 'E':  result += '\x1b'; break;

        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
        {
            u32 val = static_cast<u32>(esc - '0');
            for (int k = 1; k < 3 && i + 1 < len; ++k)
            {
                char d = str[i + 1];
                if (d < '0' || d > '7') break;
                val = val * 8 + static_cast<u32>(d - '0');
                ++i;
            }
            result += static_cast<char>(val & 0xFF);
            break;
        }

        case 'x':
        {
            if (i + 1 >= len || hexVal(str[i + 1]) < 0) { result += esc; break; }
            int val = hexVal(str[++i]);
            if (i + 1 < len && hexVal(str[i + 1]) >= 0)
                val = (val << 4) | hexVal(str[++i]);
            result += static_cast<char>(val);
            break;
        }

        case 'u':
        case 'U':
        {
            int  ndigits = (esc == 'u') ? 4 : 8;
            u32  cp      = 0;
            bool ok      = true;
            for (int k = 0; k < ndigits; ++k)
            {
                if (i + 1 >= len || hexVal(str[i + 1]) < 0) { ok = false; break; }
                cp = (cp << 4) | static_cast<u32>(hexVal(str[++i]));
            }
            if (!ok || cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF))
            {
                result += esc; break;
            }
            if      (cp <= 0x7F)   { result += static_cast<char>(cp); }
            else if (cp <= 0x7FF)  { result += static_cast<char>(0xC0 | (cp >> 6));
                                     result += static_cast<char>(0x80 | (cp & 0x3F)); }
            else if (cp <= 0xFFFF) { result += static_cast<char>(0xE0 | (cp >> 12));
                                     result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                                     result += static_cast<char>(0x80 | (cp & 0x3F)); }
            else                   { result += static_cast<char>(0xF0 | (cp >> 18));
                                     result += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
                                     result += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                                     result += static_cast<char>(0x80 | (cp & 0x3F)); }
            break;
        }

        default:
            result += '\\';
            result += esc;
            break;
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// Helper: write a single Value to a stringstream
// ---------------------------------------------------------------------------
static void writeIRValue(std::stringstream &ss, const Value &val,
                         const std::function<std::string(const std::string &)> &escape)
{
    switch (val.getType())
    {
    case ValueType::Null:
        ss << "NULL\n";
        break;
    case ValueType::Bool:
        ss << "BOOL " << (val.asBool() ? "true" : "false") << "\n";
        break;
    case ValueType::Int:
        ss << "INT " << val.asInt() << "\n";
        break;
    case ValueType::Float:
        ss << "FLOAT " << val.asFloat() << "\n";
        break;
    case ValueType::String:
        ss << "STRING \"" << escape(val.asString()) << "\"\n";
        break;

    case ValueType::Struct:
    {
        auto structPtr = val.asStruct();
        if (!structPtr)
            throw std::runtime_error("PhasorIR::serialize: null struct pointer");
        ss << "STRUCT \"" << escape(structPtr->structName) << "\" " << structPtr->fields.size() << "\n";
        for (const auto &[fieldName, fieldVal] : structPtr->fields)
        {
            ss << "  FIELD " << fieldName << " ";
            writeIRValue(ss, fieldVal, escape);
        }
        break;
    }

    case ValueType::Array:
    {
        auto arrayPtr = val.asArray();
        if (!arrayPtr)
            throw std::runtime_error("PhasorIR::serialize: null array pointer");
        ss << "ARRAY " << arrayPtr->size() << "\n";
        for (const auto &elem : *arrayPtr)
        {
            ss << "  ELEM ";
            writeIRValue(ss, elem, escape);
        }
        break;
    }

    default:
        throw std::runtime_error("PhasorIR::serialize: unknown ValueType");
    }
}

// ---------------------------------------------------------------------------
// Helper: read a single Value from a stringstream
// ---------------------------------------------------------------------------
static Value readIRValue(const std::string &type, std::stringstream &ss,
                         const std::function<std::string(const std::string &)> &unescape)
{
    if (type == "NULL")
    {
        return Value{};
    }
    else if (type == "BOOL")
    {
        std::string b;
        ss >> b;
        return Value{b == "true"};
    }
    else if (type == "INT")
    {
        i64 v;
        ss >> v;
        return Value{v};
    }
    else if (type == "FLOAT")
    {
        f64 v;
        ss >> v;
        return Value{v};
    }
    else if (type == "STRING")
    {
        std::string s;
        char        c;
        while (ss.get(c) && c != '"') {}
        std::getline(ss, s, '"');
        return Value{unescape(s)};
    }
    else if (type == "STRUCT")
    {
        std::string typeName;
        int         fieldCount = 0;

        char qc;
        while (ss.get(qc) && qc != '"') {}
        std::getline(ss, typeName, '"');
        typeName = unescape(typeName);
        ss >> fieldCount;

        auto structInstance = std::make_shared<Value::StructInstance>();
        structInstance->structName = PhsString(typeName);

        for (int f = 0; f < fieldCount; ++f)
        {
            std::string keyword, fieldName, fieldType;
            ss >> keyword;   // "FIELD"
            ss >> fieldName;
            ss >> fieldType;
            structInstance->fields[fieldName] = readIRValue(fieldType, ss, unescape);
        }

        return Value{std::move(structInstance)};
    }
    else if (type == "ARRAY")
    {
        int elementCount = 0;
        ss >> elementCount;

        auto arrayInstance = std::make_shared<Value::ArrayInstance>();
        arrayInstance->reserve(static_cast<size_t>(elementCount));

        for (int e = 0; e < elementCount; ++e)
        {
            std::string keyword, elemType;
            ss >> keyword;  // "ELEM"
            ss >> elemType;
            arrayInstance->push_back(readIRValue(elemType, ss, unescape));
        }

        return Value{std::move(arrayInstance)};
    }
    else
    {
        throw std::runtime_error("PhasorIR: unknown value type tag: " + type);
    }
}

// ---------------------------------------------------------------------------
// serialize
// ---------------------------------------------------------------------------
std::vector<u8> PhasorIR::serialize(const Bytecode &bytecode)
{
    std::stringstream ss;

    // Header
    ss << ".PHIR 3.0.0\n";

    // Build reverse lookup maps for inline comments
    std::map<int, std::string> indexToVarName;
    for (const auto &[name, index] : bytecode.variables)
        indexToVarName[index] = name;

    std::map<int, std::string> addressToFuncName;
    for (const auto &[name, address] : bytecode.functionEntries)
        addressToFuncName[address] = name;

    // Constants Section
    ss << ".CONSTANTS " << bytecode.constants.size() << "\n";
    for (const auto &val : bytecode.constants)
        writeIRValue(ss, val, escapeString);

    // Variables Section
    ss << ".VARIABLES " << bytecode.variables.size() << " " << bytecode.nextVarIndex << "\n";
    for (const auto &[name, index] : bytecode.variables)
        ss << name << " " << index << "\n";

    // Scopes Section
    ss << ".SCOPES " << bytecode.scopeVarLists.size() << "\n";
    for (const auto &varList : bytecode.scopeVarLists)
    {
        ss << varList.size();
        for (const auto &[idx, name] : varList)
            ss << " " << idx << " " << name;
        ss << "\n";
    }

    // Functions Section
    ss << ".FUNCTIONS " << bytecode.functionEntries.size() << "\n";
    for (const auto &[name, address] : bytecode.functionEntries)
        ss << name << " " << address << "\n";

    // Function Type Signatures Section
    // Format per line: <name> <returnType> <paramCount> [<paramType>...]
    ss << ".FUNCTYPES " << bytecode.functionParamTypeNames.size() << "\n";
    for (const auto &[name, paramTypes] : bytecode.functionParamTypeNames)
    {
        auto retIt = bytecode.functionReturnTypeNames.find(name);
        const std::string &retType = (retIt != bytecode.functionReturnTypeNames.end()) ? retIt->second : "any";
        ss << name << " " << retType << " " << paramTypes.size();
        for (const auto &typeName : paramTypes)
            ss << " " << typeName;
        ss << "\n";
    }

    // Structs Section
    ss << ".STRUCTS " << bytecode.structs.size() << "\n";
    for (const auto &info : bytecode.structs)
    {
        ss << info.name << " " << info.firstConstIndex << " " << info.fieldCount;
        for (const auto &fieldName : info.fieldNames)
            ss << " " << fieldName;
        ss << "\n";
    }

    // Instructions Section
    ss << ".INSTRUCTIONS " << bytecode.instructions.size() << "\n";
    for (const auto &instr : bytecode.instructions)
    {
        std::stringstream instrLine;
        instrLine << opCodeToString(instr.op);

        int     operandCount       = getOperandCount(instr.op);
        i32     operands[3]        = {instr.operand1, instr.operand2, instr.operand3};
        std::string comment;

        for (int i = 0; i < operandCount; ++i)
        {
            OperandType type = getOperandType(instr.op, i);

            if (i > 0) instrLine << ",";
            instrLine << " ";

            switch (type)
            {
            case OperandType::REGISTER:
                instrLine << "r" << operands[i];
                break;
            case OperandType::CONSTANT_IDX:
                instrLine << operands[i];
                if (operands[i] >= 0 && std::cmp_less(operands[i], bytecode.constants.size()))
                {
                    const Value &v = bytecode.constants[operands[i]];
                    if (v.getType() == ValueType::String)
                    {
                        std::string str = v.asString();
                        if (str.length() > 20) str = str.substr(0, 20) + "...";
                        comment = "const[" + std::to_string(operands[i]) + "]=\"" + escapeString(str) + "\"";
                    }
                    else if (v.getType() == ValueType::Int)
                        comment = "const[" + std::to_string(operands[i]) + "]=" + std::to_string(v.asInt());
                    else if (v.getType() == ValueType::Float)
                        comment = "const[" + std::to_string(operands[i]) + "]=" + std::to_string(v.asFloat());
                }
                break;
            case OperandType::VARIABLE_IDX:
                instrLine << operands[i];
                if (indexToVarName.contains(operands[i]))
                    comment = "var=" + indexToVarName[operands[i]];
                break;
            case OperandType::FUNCTION_IDX:
                instrLine << operands[i];
                if (addressToFuncName.contains(operands[i]))
                    comment = "func=" + addressToFuncName[operands[i]];
                break;
            default:
                instrLine << operands[i];
                break;
            }
        }

        std::string lineStr = instrLine.str();
        if (!comment.empty())
        {
            const size_t commentColumn = 40;
            if (lineStr.length() < commentColumn)
                lineStr.append(commentColumn - lineStr.length(), ' ');
            else
                lineStr += " ";
            lineStr += "; " + comment;
        }
        ss << lineStr << "\n";
    }

    std::string     textData = ss.str();
    std::vector<u8> buffer;
    buffer.insert(buffer.end(), textData.begin(), textData.end());
    return buffer;
}

// ---------------------------------------------------------------------------
// deserialize
// ---------------------------------------------------------------------------
Bytecode PhasorIR::deserialize(const std::vector<u8> &data)
{
    if (data.size() < 8)
        throw std::runtime_error("Invalid Phasor IR file: too small");

    std::string       textData(data.begin() + 8, data.end());
    std::stringstream ss(textData);
    std::string       section;
    Bytecode          bytecode;

    while (ss >> section)
    {
        if (section == ".PHIR")
        {
            std::string version;
            ss >> version;
            if (version < "3.0.0")
                throw std::runtime_error("Incompatible Phasor IR version");
        }
        else if (section == ".CONSTANTS")
        {
            int count;
            ss >> count;
            bytecode.constants.reserve(count);
            for (int i = 0; i < count; ++i)
            {
                std::string type;
                ss >> type;
                bytecode.constants.push_back(readIRValue(type, ss, unescapeString));
            }
        }
        else if (section == ".VARIABLES")
        {
            int count;
            ss >> count >> bytecode.nextVarIndex;
            for (int i = 0; i < count; ++i)
            {
                std::string name;
                int         index;
                ss >> name >> index;
                bytecode.variables[name] = index;
            }
        }
        else if (section == ".SCOPES")
        {
            int count;
            ss >> count;
            bytecode.scopeVarLists.resize(count);
            for (int i = 0; i < count; ++i)
            {
                int varCount;
                ss >> varCount;
                bytecode.scopeVarLists[i].reserve(varCount);
                for (int j = 0; j < varCount; ++j)
                {
                    int idx;
                    std::string name;
                    ss >> idx >> name;
                    bytecode.scopeVarLists[i].push_back({idx, name});
                }
            }
        }
        else if (section == ".FUNCTIONS")
        {
            int count;
            ss >> count;
            for (int i = 0; i < count; ++i)
            {
                std::string name;
                int         address;
                ss >> name >> address;
                bytecode.functionEntries[name] = address;
            }
        }
        else if (section == ".FUNCTYPES")
        {
            int count;
            ss >> count;
            for (int i = 0; i < count; ++i)
            {
                std::string name, retType;
                int         paramCount;
                ss >> name >> retType >> paramCount;

                std::vector<std::string> paramTypes;
                paramTypes.reserve(static_cast<size_t>(paramCount));
                for (int p = 0; p < paramCount; ++p)
                {
                    std::string typeName;
                    ss >> typeName;
                    paramTypes.push_back(std::move(typeName));
                }

                bytecode.functionParamTypeNames[name]  = std::move(paramTypes);
                bytecode.functionReturnTypeNames[name]  = std::move(retType);
                bytecode.functionParamCounts[name]      = paramCount;
            }
        }
        else if (section == ".STRUCTS")
        {
            int count;
            ss >> count;
            for (int i = 0; i < count; ++i)
            {
                StructInfo info;
                ss >> info.name >> info.firstConstIndex >> info.fieldCount;
                info.fieldNames.clear();
                for (int f = 0; f < info.fieldCount; ++f)
                {
                    std::string fieldName;
                    ss >> fieldName;
                    info.fieldNames.push_back(fieldName);
                }
                int index = static_cast<int>(bytecode.structs.size());
                bytecode.structs.push_back(std::move(info));
                bytecode.structEntries[bytecode.structs.back().name] = index;
            }
        }
        else if (section == ".INSTRUCTIONS")
        {
            int count;
            ss >> count;
            bytecode.instructions.reserve(count);
            for (int i = 0; i < count; ++i)
            {
                std::string opStr;
                ss >> opStr;

                OpCode  op           = stringToOpCode(opStr);
                int     operandCount = getOperandCount(op);
                i32     operands[3]  = {0, 0, 0};

                for (int j = 0; j < operandCount; ++j)
                {
                    std::string token;
                    ss >> token;

                    if (token.empty() || token[0] == ';')
                    {
                        std::getline(ss, token);
                        break;
                    }

                    if (!token.empty() && token.back() == ',')
                        token.pop_back();

                    if (!token.empty() && token[0] == 'r')
                        operands[j] = std::stoi(token.substr(1));
                    else
                        operands[j] = std::stoi(token);
                }

                // Skip any remaining content on the line (comments)
                char c;
                while (ss.get(c) && c != '\n') {}

                bytecode.instructions.emplace_back(op, operands[0], operands[1], operands[2]);
            }
        }
    }

    return bytecode;
}

// ---------------------------------------------------------------------------
// saveToFile / loadFromFile
// ---------------------------------------------------------------------------
bool PhasorIR::saveToFile(const Bytecode &bytecode, const std::filesystem::path &filename)
{
    try
    {
        std::vector<u8> data = serialize(bytecode);
        std::ofstream   file(filename, std::ios::binary);
        if (!file.is_open())
            return false;
        file.write(reinterpret_cast<const char *>(data.data()), (std::streamsize)data.size());
        return true;
    }
    catch (...)
    {
        return false;
    }
}

Bytecode PhasorIR::loadFromFile(const std::filesystem::path &filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        throw std::runtime_error("Cannot open file");

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<u8> buffer(size);
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
        throw std::runtime_error("Cannot read file");

    return deserialize(buffer);
}

} // namespace Phasor