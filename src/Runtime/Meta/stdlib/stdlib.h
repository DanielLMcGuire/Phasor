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
        PULSAR_STD_API int  length(const std::string& s);
        PULSAR_STD_API std::string  to_upper(const std::string& s);
        PULSAR_STD_API std::string  to_lower(const std::string& s);
        PULSAR_STD_API std::string  merge(const std::string& s1, const std::string& s2);
    }

    namespace math { // std_math.cpp
        PULSAR_STD_API int  add(int a, int b);
        PULSAR_STD_API int  subtract(int a, int b);
        PULSAR_STD_API int  multiply(int a, int b);
        PULSAR_STD_API int  divide(int a, int b);
        PULSAR_STD_API int  modulus(int a, int b);
        PULSAR_STD_API int  power(int base, int exp);
        PULSAR_STD_API int  abs(int a);
        PULSAR_STD_API int  min(int a, int b);
        PULSAR_STD_API int  max(int a, int b);
        PULSAR_STD_API int  factorial(int n);
        PULSAR_STD_API int  sqrt(int a);
    }

    namespace fs { // std_fs.cpp
        PULSAR_STD_API bool  change_directory(const std::string& path);
        PULSAR_STD_API void  print_current_directory();
        PULSAR_STD_API void  list_directory(const std::filesystem::path& dir = ".");
    }
    namespace file { // std_file.cpp
        PULSAR_STD_API bool  file_exists(const std::string& filename);
        PULSAR_STD_API std::string  read_line(const std::string& filename, int lineNumber);
        PULSAR_STD_API int  append_line(const std::string& filename, const std::string& newLine);
        PULSAR_STD_API int  change_line(const std::string& filename, int lineNumber, const std::string& newLine);
        PULSAR_STD_API int  delete_file(const std::string& filename);
        PULSAR_STD_API int  move_file(const std::string& oldPath, const std::string& newPath);
    }

    namespace system { // std_sys.cpp
        PULSAR_STD_API void  print_error(const std::string& message);
        PULSAR_STD_API int  execute(const std::string& command);
        PULSAR_STD_API void  wait_for_enter();
        PULSAR_STD_API void  clear_screen();
        PULSAR_STD_API std::string  get_input();
    }
} 
