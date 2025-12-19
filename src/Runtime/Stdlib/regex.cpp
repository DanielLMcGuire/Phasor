#include "StdLib.hpp"
#include <regex>
#include <vector>

Value StdLib::registerRegexFunctions(const std::vector<Value> &args, VM *vm)
{
    checkArgCount(args, 0, "include_stdregex");
    
    // Register regex functions
    vm->registerNativeFunction("regex_match", regex_match);
    vm->registerNativeFunction("regex_search", regex_search);
    vm->registerNativeFunction("regex_findall", regex_findall);
    vm->registerNativeFunction("regex_split", regex_split);
    vm->registerNativeFunction("regex_replace", regex_replace);
    
    return true;
}

Value StdLib::regex_match(const std::vector<Value> &args, VM *vm) {
    // args[1] - pattern
    // args[2] - text
    checkArgCount(args, 2, "regex_match");
    
    try {
        const std::string& pattern = args[1].asString();
        const std::string& text = args[2].asString();
        
        std::regex re(pattern);
        bool match = std::regex_match(text, re);
        return Value(match);
    } catch (const std::regex_error& e) {
        throw std::runtime_error("Regex error in regex_match: " + std::string(e.what()));
    }
}

Value StdLib::regex_search(const std::vector<Value> &args, VM *vm) {
    // args[1] - pattern
    // args[2] - text
    // args[3] - start (optional)
    // args[4] - end (optional)
    checkArgCount(args, 2, "regex_search", true);
    
    try {
        const std::string& pattern = args[1].asString();
        std::string text = args[2].asString();
        
        // Handle optional start/end positions
        size_t start_pos = 0;
        size_t end_pos = text.length();
        
        if (args.size() > 3 && !args[3].isNull()) {
            start_pos = static_cast<size_t>(args[3].asInt());
        }
        
        if (args.size() > 4 && !args[4].isNull()) {
            end_pos = static_cast<size_t>(args[4].asInt());
        }
        
        if (start_pos > end_pos || end_pos > text.length()) {
            throw std::out_of_range("Invalid start/end positions in regex_search");
        }
        
        std::regex re(pattern);
        std::smatch match;
        std::string search_text = text.substr(start_pos, end_pos - start_pos);
        
        bool found = std::regex_search(search_text, match, re);
        
        // Return the first match if found, otherwise return empty string
        return found ? Value(match[0].str()) : Value("");
        
    } catch (const std::regex_error& e) {
        throw std::runtime_error("Regex error in regex_search: " + std::string(e.what()));
    } catch (const std::out_of_range& e) {
        throw std::runtime_error("Position out of range in regex_search: " + std::string(e.what()));
    }
}

Value StdLib::regex_findall(const std::vector<Value> &args, VM *vm) {
    // args[1] - pattern
    // args[2] - text
    checkArgCount(args, 2, "regex_findall");
    
    try {
        const std::string& pattern = args[1].asString();
        const std::string& text = args[2].asString();
        
        std::regex re(pattern);
        std::sregex_iterator it(text.begin(), text.end(), re);
        std::sregex_iterator end;
        
        // Create a struct to hold the array of matches
        Value result = Value::createStruct("RegexMatches");
        int count = 0;
        
        while (it != end) {
            // Create a struct for each match
            Value match = Value::createStruct("RegexMatch");
            match.setField("text", Value(it->str()));
            match.setField("position", Value(static_cast<int64_t>(it->position())));
            
            // Add match to results
            result.setField(std::to_string(count++), match);
            ++it;
        }
        
        // Add count field
        result.setField("count", Value(static_cast<int64_t>(count)));
        
        return result;
        
    } catch (const std::regex_error& e) {
        throw std::runtime_error("Regex error in regex_findall: " + std::string(e.what()));
    }
}

Value StdLib::regex_split(const std::vector<Value> &args, VM *vm) {
    // args[1] - pattern
    // args[2] - text
    // args[3] - max split (optional, -1 for no limit)
    checkArgCount(args, 2, "regex_split", true);
    
    try {
        const std::string& pattern = args[1].asString();
        const std::string& text = args[2].asString();
        int max_split = -1;
        
        if (args.size() > 3 && !args[3].isNull()) {
            max_split = static_cast<int>(args[3].asInt());
        }
        
        std::regex re(pattern);
        std::sregex_token_iterator it(text.begin(), text.end(), re, -1);
        std::sregex_token_iterator end;
        
        // Create a struct to hold the split parts
        Value result = Value::createStruct("SplitResult");
        std::vector<std::string> parts;
        int count = 0;
        
        while (it != end && (max_split == -1 || count < max_split)) {
            parts.push_back(it->str());
            result.setField(std::to_string(count++), Value(it->str()));
            ++it;
        }
        
        // Add the rest of the string if we hit max_split
        if (it != end && max_split != -1) {
            std::string remaining = it->str();
            while (++it != end) {
                remaining += it->str();
            }
            result.setField(std::to_string(count++), Value(remaining));
        }
        
        // Add count field
        result.setField("count", Value(static_cast<int64_t>(count)));
        
        return result;
        
    } catch (const std::regex_error& e) {
        throw std::runtime_error("Regex error in regex_split: " + std::string(e.what()));
    }
}

Value StdLib::regex_replace(const std::vector<Value> &args, VM *vm) {
    // args[1] - pattern
    // args[2] - text
    // args[3] - replacement
    // args[4] - flags (optional, default to std::regex_constants::format_default)
    checkArgCount(args, 3, "regex_replace", true);
    
    try {
        const std::string& pattern = args[1].asString();
        std::string text = args[2].asString();
        const std::string& replacement = args[3].asString();
        
        // Default flags (use explicit enum types)
        std::regex_constants::syntax_option_type syntax = std::regex_constants::ECMAScript; // Default syntax
        std::regex_constants::match_flag_type match_flags = std::regex_constants::match_default;
        std::regex_constants::match_flag_type format_flags = std::regex_constants::match_default;

        // Handle optional flags
        if (args.size() > 4 && !args[4].isNull()) {
            const std::string& flagsStr = args[4].asString();

            // Parse syntax flags (these affect regex construction)
            if (flagsStr.find('e') != std::string::npos) {
                syntax = std::regex_constants::ECMAScript;
            }
            if (flagsStr.find('a') != std::string::npos) {
                syntax = std::regex_constants::awk;
            }
            if (flagsStr.find('g') != std::string::npos) {
                syntax = std::regex_constants::grep;
            }
            if (flagsStr.find('p') != std::string::npos) {
                syntax = std::regex_constants::egrep;
            }
            // Flags that modify syntax options (case, multiline, nosubs, optimize, collate)
            if (flagsStr.find('i') != std::string::npos) {
                syntax = static_cast<std::regex_constants::syntax_option_type>(
                    static_cast<int>(syntax) | static_cast<int>(std::regex_constants::icase)
                );
            }
            if (flagsStr.find('m') != std::string::npos) {
                syntax = static_cast<std::regex_constants::syntax_option_type>(
                    static_cast<int>(syntax) | static_cast<int>(std::regex_constants::multiline)
                );
            }
            if (flagsStr.find('n') != std::string::npos) {
                syntax = static_cast<std::regex_constants::syntax_option_type>(
                    static_cast<int>(syntax) | static_cast<int>(std::regex_constants::nosubs)
                );
            }
            if (flagsStr.find('o') != std::string::npos) {
                syntax = static_cast<std::regex_constants::syntax_option_type>(
                    static_cast<int>(syntax) | static_cast<int>(std::regex_constants::optimize)
                );
            }
            if (flagsStr.find('c') != std::string::npos) {
                syntax = static_cast<std::regex_constants::syntax_option_type>(
                    static_cast<int>(syntax) | static_cast<int>(std::regex_constants::collate)
                );
            }

            // Format flags for regex_replace
            if (flagsStr.find('f') != std::string::npos) {
                format_flags = static_cast<std::regex_constants::match_flag_type>(
                    static_cast<int>(format_flags) | static_cast<int>(std::regex_constants::format_sed)
                );
            }
            if (flagsStr.find('r') != std::string::npos) {
                format_flags = static_cast<std::regex_constants::match_flag_type>(
                    static_cast<int>(format_flags) | static_cast<int>(std::regex_constants::format_no_copy)
                );
            }
            if (flagsStr.find('d') != std::string::npos) {
                format_flags = static_cast<std::regex_constants::match_flag_type>(
                    static_cast<int>(format_flags) | static_cast<int>(std::regex_constants::format_first_only)
                );
            }
        }
        
        std::regex re(pattern, syntax);
        // Combine match_flags and format_flags into a single match_flag_type value
        int combined = static_cast<int>(match_flags) | static_cast<int>(format_flags);
        std::string result = std::regex_replace(
            text,
            re,
            replacement,
            static_cast<std::regex_constants::match_flag_type>(combined)
        );
        
        return Value(result);
        
    } catch (const std::regex_error& e) {
        throw std::runtime_error("Regex error in regex_replace: " + std::string(e.what()));
    }
}