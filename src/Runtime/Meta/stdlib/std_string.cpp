// PULSAR STANDARD LIBRARY IMPLEMENTATION -- stdlib/std_string.cpp -- Standard String Operations
// Author: Daniel McGuire 

#include "stdlib.h"

int PULSARSTD_api::string::length(const std::string& s) {
    return static_cast<int>(s.size());
}

std::string PULSARSTD_api::string::to_upper(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::string PULSARSTD_api::string::to_lower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string PULSARSTD_api::string::merge(const std::string& s1, const std::string& s2) {
    return s1 + s2;
}