#pragma once
// PULSAR STANDARD LIBRARY IMPLEMENTATION -- stdlib/stdlib.h
// Author: Daniel McGuire 

#pragma once

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <limits>
#include <cstdio>

#ifndef _WIN32
    #include<unistd.h>
#endif

#if defined(_WIN32) && defined(PHASOR_STD_BUILD)
    #define PULSAR_STD_API __declspec(dllexport)
#elif defined(_WIN32)
    #define PULSAR_STD_API __declspec(dllimport)
#else
    #define PULSAR_STD_API
#endif

namespace PULSARSTD_api {
    namespace string { // std_string.cpp
        PULSAR_STD_API int __fastcall length(const std::string& s);
        PULSAR_STD_API std::string __fastcall to_upper(const std::string& s);
        PULSAR_STD_API std::string __fastcall to_lower(const std::string& s);
        PULSAR_STD_API std::string __fastcall merge(const std::string& s1, const std::string& s2);
    }

    namespace math { // std_math.cpp
        PULSAR_STD_API int __fastcall add(int a, int b);
        PULSAR_STD_API int __fastcall subtract(int a, int b);
        PULSAR_STD_API int __fastcall multiply(int a, int b);
        PULSAR_STD_API int __fastcall divide(int a, int b);
        PULSAR_STD_API int __fastcall modulus(int a, int b);
        PULSAR_STD_API int __fastcall power(int base, int exp);
        PULSAR_STD_API int __fastcall abs(int a);
        PULSAR_STD_API int __fastcall min(int a, int b);
        PULSAR_STD_API int __fastcall max(int a, int b);
        PULSAR_STD_API int __fastcall factorial(int n);
        PULSAR_STD_API int __fastcall sqrt(int a);
    }
    namespace vector { // std_vector.cpp
        PULSAR_STD_API int __fastcall encode(int base, int slot, int valueToAdd, int bitsPerValue = 4);
        PULSAR_STD_API int __fastcall decode(int base, int slot, int bitsPerValue = 4);
        PULSAR_STD_API int __fastcall sort(int base, int bitsPerValue, int numSlots);
        PULSAR_STD_API int __fastcall reverse(int base, int bitsPerValue, int numSlots);
        namespace helper {
            std::vector<int> __fastcall unpack(int base, int bitsPerValue, int numSlots);
            int  __fastcall pack(const std::vector<int>& values, int bitsPerValue);
            void __fastcall debugBits(int base, int bitsPerValue, int numSlots);
        }   
    }

    namespace fs { // std_fs.cpp
        PULSAR_STD_API bool __fastcall change_directory(const std::string& path);
        PULSAR_STD_API void __fastcall print_current_directory();
        PULSAR_STD_API void __fastcall list_directory(const std::filesystem::path& dir = ".");
    }
    namespace file { // std_file.cpp
        PULSAR_STD_API bool __fastcall file_exists(const std::string& filename);
        PULSAR_STD_API std::string __fastcall read_line(const std::string& filename, int lineNumber);
        PULSAR_STD_API int __fastcall append_line(const std::string& filename, const std::string& newLine);
        PULSAR_STD_API int __fastcall change_line(const std::string& filename, int lineNumber, const std::string& newLine);
        PULSAR_STD_API int __fastcall delete_file(const std::string& filename);
        PULSAR_STD_API int __fastcall move_file(const std::string& oldPath, const std::string& newPath);
    }

    namespace system { // std_sys.cpp
        PULSAR_STD_API void __fastcall print_error(const std::string& message);
        PULSAR_STD_API int __fastcall execute(const std::string& command);
        PULSAR_STD_API void __fastcall wait_for_enter();
        PULSAR_STD_API void __fastcall clear_screen();
        PULSAR_STD_API std::string __fastcall get_input();
    }
} 
