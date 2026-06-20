#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>

const std::vector<std::string> PATCHES = {
    "#pragma warning(disable:4996)",
    "#pragma warning(disable:4244)"
};

// --- TYPE REGISTRY ---
struct Member {
    std::string type;
    std::string name;
};

struct StructDef {
    std::string name;
    std::vector<Member> members;
};

std::map<std::string, StructDef> StructRegistry = {
    {"POINT", {"POINT", {{"LONG", "x"}, {"LONG", "y"}}}},
    {"RECT",  {"RECT",  {{"LONG", "left"}, {"LONG", "top"}, {"LONG", "right"}, {"LONG", "bottom"}}}},
    {"WNDCLASSEXW", {"WNDCLASSEXW", {
        {"UINT", "cbSize"},
        {"UINT", "style"},
        {"LONG", "lpfnWndProc"},
        {"int", "cbClsExtra"},
        {"int", "cbWndExtra"},
        {"HINSTANCE", "hInstance"},
        {"HICON", "hIcon"},
        {"HCURSOR", "hCursor"},
        {"HBRUSH", "hbrBackground"},
        {"LPCWSTR", "lpszMenuName"},
        {"LPCWSTR", "lpszClassName"},
        {"HICON", "hIconSm"}
    }}}
};

// --- INTERNAL DATA STRUCTURES ---
struct ParamInfo {
    std::string cppType;      
    std::string varName;      
    int expectedType;         
    bool isStruct = false;
    std::string structName;   
    bool isPointer = false;   
    bool allowNull = false;   
};

struct Function {
    std::string returnType;
    std::string name;
    std::vector<ParamInfo> params;
    bool isReturnStruct = false;
    std::string returnStructName;
    std::string rawLine;
    int lineNumber = 0;
};

// --- HELPERS ---

bool isHandleType(const std::string &type) {
    return type == "HANDLE" || type == "HMODULE" || type == "HWND" || 
           type == "HINSTANCE" || type == "HDC" || type == "HMENU" || 
           type == "LPVOID" || type == "WPARAM" || type == "LPARAM";
}

std::string trim(const std::string &str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

ParamInfo resolveParam(std::string type, std::string name, [[maybe_unused]] int index) {
    ParamInfo p;
    p.varName = name;
    p.allowNull = false;

    if (type.substr(0, 2) == "LP") {
        std::string base = type.substr(2);
        if (StructRegistry.count(base)) {
            p.isStruct = true;
            p.structName = base;
            p.isPointer = true;
            p.cppType = base + "*";
            p.expectedType = 5; 
            p.allowNull = true; 
            return p;
        }
    }
    
    if (type.size() > 6 && type.substr(0, 6) == "const ") {
        std::string base = type.substr(6);
        if (base.back() == '*') base.pop_back();
        if (StructRegistry.count(base)) {
            p.isStruct = true;
            p.structName = base;
            p.isPointer = true;
            p.cppType = type;
            p.expectedType = 5;
            p.allowNull = true;
            return p;
        }
    }

    if (isHandleType(type)) {
        p.cppType = type;
        p.expectedType = 2; 
        p.allowNull = true;
    } else if (type == "BOOL") {
        p.cppType = "BOOL";
        p.expectedType = 1; 
    } else if (type == "DWORD" || type == "int" || type == "LONG" || type == "UINT" || type == "ULONG") {
        p.cppType = type;
        p.expectedType = 2; 
    } else if (type == "float" || type == "double") {
        p.cppType = type;
        p.expectedType = 3; 
    } else if (type == "LPCSTR" || type == "LPSTR" || type == "constchar*") {
        p.cppType = "char*";
        p.expectedType = 4; 
        p.allowNull = true;
    } else if (type == "LPCWSTR" || type == "LPWSTR" || type == "constwchar_t*") {
        p.cppType = "wchar_t*";
        p.expectedType = 4; 
        p.allowNull = true;
    } else {
        p.cppType = type;
        p.expectedType = 0; 
    }

    return p;
}

Function parseFunction(const std::string &line, int lineNumber) {
    Function f;
    f.rawLine = line;
    f.lineNumber = lineNumber;
    std::string clean = line;
    std::erase(clean, ';');
    size_t paren = clean.find('(');
    if (paren == std::string::npos) return f;

    std::string retAndName = clean.substr(0, paren);
    std::string paramStr = clean.substr(paren + 1, clean.find(')') - paren - 1);
    std::istringstream iss(retAndName);
    iss >> f.returnType >> f.name;

    if (StructRegistry.count(f.returnType)) {
        f.isReturnStruct = true;
        f.returnStructName = f.returnType;
    }

    paramStr = trim(paramStr);
    if (paramStr.empty() || paramStr == "void") return f;

    std::istringstream pstream(paramStr);
    std::string segment;
    int idx = 0;
    while (std::getline(pstream, segment, ',')) {
        segment = trim(segment);
        if (segment.empty()) continue;
        std::istringstream ps(segment);
        std::string t, n;
        ps >> t >> n;
        f.params.push_back(resolveParam(t, n, idx++));
    }
    return f;
}

// --- GENERATOR ---

void generateWrapper(const Function &f, std::ostream &out) {
    if (f.returnType.empty()) return;

    out << "// " << f.rawLine << " @ln:" << f.lineNumber << "\n";
    out << "static PhasorValue win32_" << f.name
        << "([[maybe_unused]] PhasorVM* vm, [[maybe_unused]] int argc, [[maybe_unused]] const PhasorValue* argv) {\n";

    for (size_t i = 0; i < f.params.size(); ++i) {
        const auto &p = f.params[i];
        out << "    ";

        if (p.isStruct) {
            out << p.structName << " local_" << p.varName << ";\n";
            out << "    {\n";
            for (const auto& m : StructRegistry.at(p.structName).members) {
                out << "        for (size_t k = 0; k < argv[" << i << "].as.st.count; ++k) {\n";
                out << "            if (strcmp(argv[" << i << "].as.st.keys[k], \"" << m.name << "\") == 0) {\n";
                
                if (m.type == "LPCWSTR") {
                    out << "                local_" << p.varName << "." << m.name << " = (LPCWSTR)phasor_to_string(argv[" << i << "].as.st.values[k]);\n";
                } else if (m.type == "LONG" || m.type == "DWORD" || m.type == "UINT" || m.type == "HINSTANCE" || m.type == "HICON" || m.type == "HCURSOR" || m.type == "HBRUSH") {
                    out << "                local_" << p.varName << "." << m.name << " = (" << m.type << ")phasor_to_int(argv[" << i << "].as.st.values[k]);\n";
                } else {
                    out << "                local_" << p.varName << "." << m.name << " = (" << m.type << ")phasor_to_float(argv[" << i << "].as.st.values[k]);\n";
                }
                
                out << "                break;\n            }\n        }\n";
            }
            out << "    }\n";
        } else {
            if (p.allowNull) {
                if (p.cppType == "char*" || p.cppType == "wchar_t*") {
                    out << "if (!phasor_is_null(argv[" << i << "]) && !phasor_is_string(argv[" << i << "])) throw std::runtime_error(\"Arg " << i << ": expected string or null\");\n";
                    out << "    " << p.cppType << " " << p.varName << " = phasor_is_null(argv[" << i << "]) ? nullptr : (" << p.cppType << ")phasor_to_string(argv[" << i << "]);\n";
                } else {
                    out << "if (!phasor_is_null(argv[" << i << "]) && !phasor_is_int(argv[" << i << "])) throw std::runtime_error(\"Arg " << i << ": expected int or null\");\n";
                    out << "    " << p.cppType << " " << p.varName << " = phasor_is_null(argv[" << i << "]) ? nullptr : (" << p.cppType << ")phasor_to_int(argv[" << i << "]);\n";
                }
            } else {
                out << "if (phasor_is_null(argv[" << i << "])) throw std::runtime_error(\"Arg " << i << ": expected " << p.cppType << ", got null\");\n";
                out << "    ";
                
                // FIX: Added safety check to prevent "phasor_is_"
                std::string typeCheckFunc = "phasor_is_int"; // Default fallback
                if (p.expectedType == 1) typeCheckFunc = "phasor_is_bool";
                else if (p.expectedType == 2) typeCheckFunc = "phasor_is_int";
                else if (p.expectedType == 3) typeCheckFunc = "phasor_is_float";
                else if (p.expectedType == 4) typeCheckFunc = "phasor_is_string";
                else if (p.expectedType == 5) typeCheckFunc = "phasor_is_struct";

                out << "if (!" << typeCheckFunc << "(argv[" << i << "])) throw std::runtime_error(\"Arg " << i << ": type mismatch\");\n";
                out << "    ";

                if (p.cppType == "char*" || p.cppType == "wchar_t*") {
                    out << p.cppType << " " << p.varName << " = (" << p.cppType << ")phasor_to_string(argv[" << i << "]);\n";
                } else if (p.expectedType == 3) {
                    out << p.cppType << " " << p.varName << " = (" << p.cppType << ")phasor_to_float(argv[" << i << "]);\n";
                } else {
                    out << p.cppType << " " << p.varName << " = (" << p.cppType << ")phasor_to_int(argv[" << i << "]);\n";
                }
            }
        }
    }

    out << "    auto result = " << f.name << "(";
    for (size_t i = 0; i < f.params.size(); ++i) {
        if (i != 0) out << ", ";
        if (f.params[i].isStruct) out << "&local_" << f.params[i].varName;
        else out << f.params[i].varName;
    }
    out << ");\n";

    if (f.isReturnStruct) {
        const auto& sd = StructRegistry.at(f.returnStructName);
        out << "    const char* r_keys[] = {";
        for (size_t i=0; i<sd.members.size(); ++i) out << "\"" << sd.members[i].name << "\"" << (i == sd.members.size()-1 ? "" : ", ");
        out << "};\n    PhasorValue r_vals[] = {";
        for (const auto& m : sd.members) {
            out << (m.type == "LONG" ? "phasor_make_int(result." : "phasor_make_float(result.") << m.name << "), ";
        }
        out << "};\n    return phasor_make_struct(\"" << f.returnStructName << "\", r_keys, r_vals, " << sd.members.size() << ");\n";
    } else if (isHandleType(f.returnType)) {
        out << "    return phasor_make_int((int64_t)result);\n";
    } else if (f.returnType == "BOOL") {
        out << "    return phasor_make_bool(result);\n";
    } else if (f.returnType == "DWORD" || f.returnType == "int" || f.returnType == "LONG" || f.returnType == "UINT" || f.returnType == "ULONG") {
        out << "    return phasor_make_int(result);\n";
    } else if (f.returnType == "float" || f.returnType == "double") {
        out << "    return phasor_make_float(result);\n";
    } else {
        out << "    return phasor_make_null();\n";
    }

    out << "}\n\n";
}

int main(int argc, char **argv) {
    std::string inputFile = "winapi.h";
    std::string outputFile = "phasor_winapi.cpp";
    if (argc >= 2) inputFile = argv[1];
    if (argc >= 4 && std::string(argv[2]) == "-o") outputFile = argv[3];

    std::ifstream infile(inputFile);
    std::ofstream outfile(outputFile);

    outfile << "#define PHASOR_FFI_BUILD_DLL\n#include <PhasorFFI.h>\n#include <windows.h>\n#include <stdexcept>\n#include \"../src/Bindings/win32/handle.hpp\"\n\n";
    outfile << "// =====BEGIN PATCHES=====\n";
    for (const auto &patch : PATCHES) outfile << patch << "\n";
    outfile << "// ======END PATCHES======\n\n";

    std::string line;
    int lineNum = 0;
    std::vector<Function> funcs;
    while (std::getline(infile, line)) {
        lineNum++;
        if (line.empty() || line.find("//") == 0) continue;
        Function f = parseFunction(line, lineNum);
        if (!f.returnType.empty()) funcs.push_back(f);
    }

    for (const auto &f : funcs) generateWrapper(f, outfile);

    outfile << "PHASOR_FFI_EXPORT void phasor_plugin_entry(const PhasorAPI* api, PhasorVM* vm) {\n";
    for (const auto &f : funcs) outfile << "    api->register_function(vm, \"win32_" << f.name << "\", win32_" << f.name << ");\n";
    outfile << "}\n";
    return 0;
}