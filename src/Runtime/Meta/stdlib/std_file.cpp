// PULSAR STANDARD LIBRARY IMPLEMENTATION -- stdlib/std_file.cpp -- Standard File Operations
// Author: Daniel McGuire 

#include "stdlib.h"

std::string PULSARSTD_api::file::read_line(const std::string& filename, int lineNumber) {
    std::ifstream file(filename);
    std::string line;
    int currentLine = 1;
    while (std::getline(file, line)) {
        if (currentLine == lineNumber) {
            return line;
        }
        currentLine++;
    }
    return "";
}

int PULSARSTD_api::file::append_line(const std::string& filename, const std::string& newLine) {
    std::ofstream file(filename, std::ios::app);
    file << newLine << "\n";
    return 0;
}

int PULSARSTD_api::file::change_line(const std::string& filename, int lineNumber, const std::string& newLine) {
    std::ifstream file(filename);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
        }
    file.close();

    if (lineNumber > 0 && lineNumber <= static_cast<int>(lines.size())) {
        lines[lineNumber - 1] = newLine;
    }
    else {
        throw std::runtime_error("Line number out of range: " + std::to_string(lineNumber));
    }
    std::ofstream outFile(filename, std::ios::trunc);
    for (const auto& l : lines) {
        outFile << l << "\n";
    }
    return 0;
}

bool PULSARSTD_api::file::file_exists(const std::string& path) {
    return std::filesystem::exists(std::filesystem::u8path(path));
}

int PULSARSTD_api::file::delete_file(const std::string& filename) {
    if (std::filesystem::remove(std::filesystem::u8path(filename))) {
        return 0;
    } else {
        throw std::runtime_error("Failed to delete file: " + filename);
    }
}

int PULSARSTD_api::file::move_file(const std::string& oldPath, const std::string& newPath) {
    try {
        std::filesystem::rename(std::filesystem::u8path(oldPath), std::filesystem::u8path(newPath));
        return 0;
    } catch (const std::filesystem::filesystem_error& e) {
        throw std::runtime_error("Failed to move file: " + std::string(e.what()));
    }
}