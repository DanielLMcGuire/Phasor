// PULSAR STANDARD LIBRARY IMPLEMENTATION -- stdlib/std_fs.cpp -- Standard File System Operations
// Author: Daniel McGuire 

#include "stdlib.h"

bool PULSARSTD_api::fs::change_directory(const std::string& path) {
    try {
        std::filesystem::current_path(path);  
        return true;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error changing directory: " << e.what() << '\n';
        return false;
    }
}  

void PULSARSTD_api::fs::print_current_directory() {
    try {
        std::filesystem::path cwd = std::filesystem::current_path();
        std::cout << "Current working directory: " << cwd << '\n';
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error getting current directory: " << e.what() << '\n';
    }
}

void PULSARSTD_api::fs::list_directory(const std::filesystem::path& dir) {
    try {
        if (!std::filesystem::exists(dir)) {
            std::cerr << "Directory does not exist: " << dir << '\n';
            return;
        }
        if (!std::filesystem::is_directory(dir)) {
            std::cerr << "Not a directory: " << dir << '\n';
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            std::cout << entry.path().filename().string();

            if (std::filesystem::is_directory(entry.status())) {
                std::cout << "/";
            }
            std::cout << '\n';
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << '\n';
    }
}