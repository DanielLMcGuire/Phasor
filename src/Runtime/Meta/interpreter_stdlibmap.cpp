#include "interpreter.hpp"
#include "stdlib/stdlib.h"
static std::unordered_map<std::string, std::function<pulsar::interpreter::Token(const std::vector<pulsar::interpreter::Token>&)>> standardLibraryFunctions = {
    
    // FILE SYSTEM OPERATIONS
    { 
        "cd", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 1) {
                throw std::runtime_error("change_directory: Expected exactly 1 argument, got " + std::to_string(args.size()) + 
                                       ". Usage: change_directory \"path\"");
            }
	        if (args[0].type != pulsar::interpreter::TokenType::STRING)
	        {
                throw std::runtime_error("change_directory: Argument must be a string (directory path). "
                                       "Usage: change_directory \"/path/to/directory\"");
            }
            
            int result = PULSARSTD_api::fs::change_directory(args[0].value);
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    
    { 
        "pcd", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (!args.empty()) {
                throw std::runtime_error("print_current_directory: Expected no arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: print_current_directory");
            }
            
            PULSARSTD_api::fs::print_current_directory();
            return pulsar::interpreter::Token{ std::to_string(0), pulsar::interpreter::TokenType::INT };
        }
    },
    { 
        "ls", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() > 1) {
                throw std::runtime_error("list_directory: Expected at most 1 argument, got " + std::to_string(args.size()) + 
                                          ". Usage: list_directory [\"optional_path\"]");
            }
	        if (args.size() == 1 && args[0].type != pulsar::interpreter::TokenType::STRING)
	        {
                throw std::runtime_error("list_directory: Argument must be a string (directory path). " 
                                            "Usage: list_directory [\"optional_path\"]");
            }
            std::string path = (args.size() == 1) ? args[0].value : ".";
            PULSARSTD_api::fs::list_directory(path);
            return pulsar::interpreter::Token{ std::to_string(0), pulsar::interpreter::TokenType::INT };
        }
    },

    // FILE OPERATIONS
    { 
        "rl", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 2) {
                throw std::runtime_error("file_read_line: Expected exactly 2 arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: file_read_line \"filename\" line_number");
            }
	        if (args[0].type != pulsar::interpreter::TokenType::STRING)
	        {
                throw std::runtime_error("file_read_line: First argument must be a string (filename). "
                                       "Usage: file_read_line \"filename.txt\" 1");
            }
	        if (args[1].type != pulsar::interpreter::TokenType::INT)
	        {
                throw std::runtime_error("file_read_line: Second argument must be an integer (line number). "
                                       "Usage: file_read_line \"filename.txt\" 1");
            }
            
            std::string result = PULSARSTD_api::file::read_line(args[0].value, std::stoi(args[1].value));
            return pulsar::interpreter::Token{ result, pulsar::interpreter::TokenType::STRING };
        }
    },
    
    { 
        "al", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 2) {
                throw std::runtime_error("file_append_line: Expected exactly 2 arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: file_append_line \"filename\" \"text_to_append\"");
            }
	        if (args[0].type != pulsar::interpreter::TokenType::STRING)
	        {
                throw std::runtime_error("file_append_line: First argument must be a string (filename). "
                                       "Usage: file_append_line \"filename.txt\" \"new line\"");
            }
	        if (args[1].type != pulsar::interpreter::TokenType::STRING)
	        {
                throw std::runtime_error("file_append_line: Second argument must be a string (text to append). "
                                       "Usage: file_append_line \"filename.txt\" \"new line\"");
            }
            
            int result = PULSARSTD_api::file::append_line(args[0].value, args[1].value);
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    
    { 
        "cl", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 3) {
                throw std::runtime_error("file_change_line: Expected exactly 3 arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: file_change_line \"filename\" line_number \"new_text\"");
            }
	        if (args[0].type != pulsar::interpreter::TokenType::STRING)
	        {
                throw std::runtime_error("file_change_line: First argument must be a string (filename). "
                                       "Usage: file_change_line \"filename.txt\" 1 \"replacement text\"");
            }
            if (args[1].type != pulsar::interpreter::TokenType::INT) {
                throw std::runtime_error("file_change_line: Second argument must be an integer (line number). "
                                       "Usage: file_change_line \"filename.txt\" 1 \"replacement text\"");
            }
            if (args[2].type != pulsar::interpreter::TokenType::STRING) {
                throw std::runtime_error("file_change_line: Third argument must be a string (replacement text). "
                                       "Usage: file_change_line \"filename.txt\" 1 \"replacement text\"");
            }
            
            int result = PULSARSTD_api::file::change_line(args[0].value, std::stoi(args[1].value), args[2].value);
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    {   
        "fe", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 1) {
                throw std::runtime_error("file_exists: Expected exactly 1 argument, got " + std::to_string(args.size()) + 
                                       ". Usage: file_exists \"filename\"");
            }
            if (args[0].type != pulsar::interpreter::TokenType::STRING) {
                throw std::runtime_error("file_exists: Argument must be a string (filename). "
                                       "Usage: file_exists \"filename.txt\"");
            }
            
            bool exists = PULSARSTD_api::file::file_exists(args[0].value);
            return pulsar::interpreter::Token{ exists ? "true" : "false", pulsar::interpreter::TokenType::STRING };
        }
    },
    {
        "rm", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 1) {
                throw std::runtime_error("file_delete: Expected exactly 1 argument, got " + std::to_string(args.size()) + 
                                       ". Usage: file_delete \"filename\"");
            }
            if (args[0].type != pulsar::interpreter::TokenType::STRING) {
                throw std::runtime_error("file_delete: Argument must be a string (filename). "
                                       "Usage: rm \"filename.txt\"");
            }
            int result = PULSARSTD_api::file::delete_file(args[0].value);
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    {
        "mv", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 2) {
                throw std::runtime_error("file_move: Expected exactly 2 arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: mv \"source_filename\" \"destination_filename\"");
            }
            if (args[0].type != pulsar::interpreter::TokenType::STRING || args[1].type != pulsar::interpreter::TokenType::STRING) {
                throw std::runtime_error("file_move: Both arguments must be strings (filenames). "
                                       "Usage: mv \"source.txt\" \"destination.txt\"");
            }
            int result = PULSARSTD_api::file::move_file(args[0].value, args[1].value);
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },

    // STRING OPERATIONS
    { 
        "str_len", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 1) {
                throw std::runtime_error("string_length: Expected exactly 1 argument, got " + std::to_string(args.size()) + 
                                       ". Usage: string_length \"text\"");
            }
            if (args[0].type != pulsar::interpreter::TokenType::STRING) {
                throw std::runtime_error("string_length: Argument must be a string. "
                                       "Usage: string_length \"hello world\"");
            }
            
            int result = PULSARSTD_api::string::length(args[0].value);
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    
    { 
        "str_up", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 1) {
                throw std::runtime_error("string_to_upper: Expected exactly 1 argument, got " + std::to_string(args.size()) + 
                                       ". Usage: string_to_upper \"text\"");
            }
            if (args[0].type != pulsar::interpreter::TokenType::STRING) {
                throw std::runtime_error("string_to_upper: Argument must be a string. "
                                       "Usage: string_to_upper \"hello world\"");
            }
            
            std::string result = PULSARSTD_api::string::to_upper(args[0].value);
            return pulsar::interpreter::Token{ result, pulsar::interpreter::TokenType::STRING };
        }
    },
    
    { 
        "str_low", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 1) {
                throw std::runtime_error("string_to_lower: Expected exactly 1 argument, got " + std::to_string(args.size()) + 
                                       ". Usage: string_to_lower \"TEXT\"");
            }
            if (args[0].type != pulsar::interpreter::TokenType::STRING) {
                throw std::runtime_error("string_to_lower: Argument must be a string. "
                                       "Usage: string_to_lower \"HELLO WORLD\"");
            }
            
            std::string result = PULSARSTD_api::string::to_lower(args[0].value);
            return pulsar::interpreter::Token{ result, pulsar::interpreter::TokenType::STRING };
        }
    },
    {
        "str_merge", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 2) {
                throw std::runtime_error("string_merge: Expected exactly 2 arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: string_merge \"text1\" \"text2\"");
            }
            if (args[0].type != pulsar::interpreter::TokenType::STRING || args[1].type != pulsar::interpreter::TokenType::STRING) {
                throw std::runtime_error("string_merge: Both arguments must be strings. "
                                       "Usage: string_merge \"Hello\" \"World\"");
            }
            
            std::string result = PULSARSTD_api::string::merge(args[0].value, args[1].value);
            return pulsar::interpreter::Token{ result, pulsar::interpreter::TokenType::STRING };
        }
    },

    // MATHEMATICAL OPERATIONS
    { 
        "add", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 2) {
                throw std::runtime_error("math_add: Expected exactly 2 arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: math_add number1 number2");
            }
            if (args[0].type != pulsar::interpreter::TokenType::INT || args[1].type != pulsar::interpreter::TokenType::INT) {
                throw std::runtime_error("math_add: Both arguments must be integers. "
                                       "Usage: math_add 10 5");
            }
            
            int result = PULSARSTD_api::math::add(std::stoi(args[0].value), std::stoi(args[1].value));
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    { 
        "subtract", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 2) {
                throw std::runtime_error("math_subtract: Expected exactly 2 arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: math_subtract number1 number2");
            }
            if (args[0].type != pulsar::interpreter::TokenType::INT || args[1].type != pulsar::interpreter::TokenType::INT) {
                throw std::runtime_error("math_subtract: Both arguments must be integers. "
                                       "Usage: math_subtract 10 3");
            }
            
            int result = PULSARSTD_api::math::subtract(std::stoi(args[0].value), std::stoi(args[1].value));
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    
    { 
        "multiply", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 2) {
                throw std::runtime_error("math_multiply: Expected exactly 2 arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: math_multiply number1 number2");
            }
            if (args[0].type != pulsar::interpreter::TokenType::INT || args[1].type != pulsar::interpreter::TokenType::INT) {
                throw std::runtime_error("math_multiply: Both arguments must be integers. "
                                       "Usage: math_multiply 6 7");
            }
            
            int result = PULSARSTD_api::math::multiply(std::stoi(args[0].value), std::stoi(args[1].value));
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    
    { 
        "divide", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 2) {
                throw std::runtime_error("math_divide: Expected exactly 2 arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: math_divide dividend divisor");
            }
            if (args[0].type != pulsar::interpreter::TokenType::INT || args[1].type != pulsar::interpreter::TokenType::INT) {
                throw std::runtime_error("math_divide: Both arguments must be integers. "
                                       "Usage: math_divide 20 4");
            }
            if (std::stoi(args[1].value) == 0) {
                throw std::runtime_error("math_divide: Division by zero is not allowed. "
                                       "Divisor (second argument) cannot be 0");
            }
            
            int result = PULSARSTD_api::math::divide(std::stoi(args[0].value), std::stoi(args[1].value));
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    
    { 
        "mod", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 2) {
                throw std::runtime_error("math_modulus: Expected exactly 2 arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: math_modulus dividend divisor");
            }
            if (args[0].type != pulsar::interpreter::TokenType::INT || args[1].type != pulsar::interpreter::TokenType::INT) {
                throw std::runtime_error("math_modulus: Both arguments must be integers. "
                                       "Usage: math_modulus 17 5");
            }
            if (std::stoi(args[1].value) == 0) {
                throw std::runtime_error("math_modulus: Modulus by zero is not allowed. "
                                       "Divisor (second argument) cannot be 0");
            }
            
            int result = PULSARSTD_api::math::modulus(std::stoi(args[0].value), std::stoi(args[1].value));
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    
    { 
        "exp", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 2) {
                throw std::runtime_error("math_power: Expected exactly 2 arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: math_power base exponent");
            }
            if (args[0].type != pulsar::interpreter::TokenType::INT || args[1].type != pulsar::interpreter::TokenType::INT) {
                throw std::runtime_error("math_power: Both arguments must be integers. "
                                       "Usage: math_power 2 8");
            }
            
            int result = PULSARSTD_api::math::power(std::stoi(args[0].value), std::stoi(args[1].value));
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    
    { 
        "abs", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 1) {
                throw std::runtime_error("math_absolute: Expected exactly 1 argument, got " + std::to_string(args.size()) + 
                                       ". Usage: math_absolute number");
            }
            if (args[0].type != pulsar::interpreter::TokenType::INT) {
                throw std::runtime_error("math_absolute: Argument must be an integer. "
                                       "Usage: math_absolute -42");
            }
            
            int result = PULSARSTD_api::math::abs(std::stoi(args[0].value));
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    { 
        "min", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 2) {
                throw std::runtime_error("math_minimum: Expected exactly 2 arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: math_minimum number1 number2");
            }
            if (args[0].type != pulsar::interpreter::TokenType::INT || args[1].type != pulsar::interpreter::TokenType::INT) {
                throw std::runtime_error("math_minimum: Both arguments must be integers. "
                                       "Usage: math_minimum 10 5");
            }
            
            int result = PULSARSTD_api::math::min(std::stoi(args[0].value), std::stoi(args[1].value));
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    
    { 
        "max", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 2) {
                throw std::runtime_error("math_maximum: Expected exactly 2 arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: math_maximum number1 number2");
            }
            if (args[0].type != pulsar::interpreter::TokenType::INT || args[1].type != pulsar::interpreter::TokenType::INT) {
                throw std::runtime_error("math_maximum: Both arguments must be integers. "
                                       "Usage: math_maximum 10 5");
            }
            
            int result = PULSARSTD_api::math::max(std::stoi(args[0].value), std::stoi(args[1].value));
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    
    { 
        "factor", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 1) {
                throw std::runtime_error("math_factorial: Expected exactly 1 argument, got " + std::to_string(args.size()) + 
                                       ". Usage: math_factorial number");
            }
            if (args[0].type != pulsar::interpreter::TokenType::INT) {
                throw std::runtime_error("math_factorial: Argument must be an integer. "
                                       "Usage: math_factorial 5");
            }
            int value = std::stoi(args[0].value);
            if (value < 0) {
                throw std::runtime_error("math_factorial: Argument must be non-negative. "
                                       "Factorial is undefined for negative numbers");
            }
            
            int result = PULSARSTD_api::math::factorial(value);
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    
    { 
        "sqrt", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 1) {
                throw std::runtime_error("math_square_root: Expected exactly 1 argument, got " + std::to_string(args.size()) + 
                                       ". Usage: math_square_root number");
            }
            if (args[0].type != pulsar::interpreter::TokenType::INT) {
                throw std::runtime_error("math_square_root: Argument must be an integer. "
                                       "Usage: math_square_root 16");
            }
            int value = std::stoi(args[0].value);
            if (value < 0) {
                throw std::runtime_error("math_square_root: Argument must be non-negative. "
                                       "Square root of negative numbers is not supported");
            }
            
            int result = PULSARSTD_api::math::sqrt(value);
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },

    // VECTOR OPERATIONS
    { 
        "vector_encode", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 3) {
                throw std::runtime_error("vector_encode: Expected exactly 3 arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: vector_encode base_vector slot_index value_to_store");
            }
            if (args[0].type != pulsar::interpreter::TokenType::INT || 
                args[1].type != pulsar::interpreter::TokenType::INT || 
                args[2].type != pulsar::interpreter::TokenType::INT) {
                throw std::runtime_error("vector_encode: All arguments must be integers. "
                                       "Usage: vector_encode 0 0 42");
            }

            int base = std::stoi(args[0].value);
            int slot = std::stoi(args[1].value);
            int valueToAdd = std::stoi(args[2].value);
            
            if (slot < 0) {
                throw std::runtime_error("vector_encode: Slot index must be non-negative. Got: " + std::to_string(slot));
            }

            int result = PULSARSTD_api::vector::encode(base, slot, valueToAdd);
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    
    { 
        "vector_decode", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 2) {
                throw std::runtime_error("vector_decode: Expected exactly 2 arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: vector_decode encoded_vector slot_index");
            }
            if (args[0].type != pulsar::interpreter::TokenType::INT || 
                args[1].type != pulsar::interpreter::TokenType::INT) {
                throw std::runtime_error("vector_decode: Both arguments must be integers. "
                                       "Usage: vector_decode 12345 0");
            }
            
            int base = std::stoi(args[0].value);
            int slot = std::stoi(args[1].value);
            
            if (slot < 0) {
                throw std::runtime_error("vector_decode: Slot index must be non-negative. Got: " + std::to_string(slot));
            }
            
            int result = PULSARSTD_api::vector::decode(base, slot);
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    
    { 
        "vector_sort", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 3) {
                throw std::runtime_error("vector_sort: Expected exactly 3 arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: vector_sort encoded_vector bits_per_value number_of_slots");
            }
            if (args[0].type != pulsar::interpreter::TokenType::INT || 
                args[1].type != pulsar::interpreter::TokenType::INT || 
                args[2].type != pulsar::interpreter::TokenType::INT) {
                throw std::runtime_error("vector_sort: All arguments must be integers. "
                                       "Usage: vector_sort 12345 8 4");
            }
            
            int base = std::stoi(args[0].value);
            int bitsPerValue = std::stoi(args[1].value);
            int numSlots = std::stoi(args[2].value);
            
            if (bitsPerValue <= 0) {
                throw std::runtime_error("vector_sort: Bits per value must be positive. Got: " + std::to_string(bitsPerValue));
            }
            if (numSlots <= 0) {
                throw std::runtime_error("vector_sort: Number of slots must be positive. Got: " + std::to_string(numSlots));
            }
            
            int result = PULSARSTD_api::vector::sort(base, bitsPerValue, numSlots);
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },

    { 
        "vector_reverse", 
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 3) {
                throw std::runtime_error("vector_reverse: Expected exactly 3 arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: vector_reverse encoded_vector bits_per_value number_of_slots");
            }
            if (args[0].type != pulsar::interpreter::TokenType::INT || 
                args[1].type != pulsar::interpreter::TokenType::INT || 
                args[2].type != pulsar::interpreter::TokenType::INT) {
                throw std::runtime_error("vector_reverse: All arguments must be integers. "
                                       "Usage: vector_reverse 12345 8 4");
            }
            
            int base = std::stoi(args[0].value);
            int bitsPerValue = std::stoi(args[1].value);
            int numSlots = std::stoi(args[2].value);
            
            if (bitsPerValue <= 0) {
                throw std::runtime_error("vector_reverse: Bits per value must be positive. Got: " + std::to_string(bitsPerValue));
            }
            if (numSlots <= 0) {
                throw std::runtime_error("vector_reverse: Number of slots must be positive. Got: " + std::to_string(numSlots));
            }
            
            int result = PULSARSTD_api::vector::reverse(base, bitsPerValue, numSlots);
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },

    // SYSTEM OPERATIONS
    {
        "print_err",
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 1) {
                throw std::runtime_error("print_error: Expected exactly 1 argument, got " + std::to_string(args.size()) + 
                                       ". Usage: print_error \"message\"");
            }
            if (args[0].type != pulsar::interpreter::TokenType::STRING) {
                throw std::runtime_error("print_error: Argument must be a string. "
                                       "Usage: print_error \"An error occurred\"");
            }
            
            PULSARSTD_api::system::print_error(args[0].value);
            return pulsar::interpreter::Token{ "0", pulsar::interpreter::TokenType::INT }; // success token
        }
    },
    {
        "sys_exec",
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (args.size() != 1) {
                throw std::runtime_error("system_execute: Expected exactly 1 argument, got " + std::to_string(args.size()) + 
                                       ". Usage: system_execute \"command\"");
            }
            if (args[0].type != pulsar::interpreter::TokenType::STRING) {
                throw std::runtime_error("system_execute: Argument must be a string (command). "
                                       "Usage: system_execute \"ls -l\"");
            }
            
            int result = PULSARSTD_api::system::execute(args[0].value);
            return pulsar::interpreter::Token{ std::to_string(result), pulsar::interpreter::TokenType::INT };
        }
    },
    {
        "sys_wait",
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (!args.empty()) {
                throw std::runtime_error("system_pause: Expected no arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: system_pause");
            }
            
            PULSARSTD_api::system::wait_for_enter();
            return pulsar::interpreter::Token{ "0", pulsar::interpreter::TokenType::INT }; // success token
        }
    },
    {
        "clear",
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (!args.empty()) {
                throw std::runtime_error("console_clear: Expected no arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: console_clear");
            }
            
            PULSARSTD_api::system::clear_screen();
            return pulsar::interpreter::Token{ "0", pulsar::interpreter::TokenType::INT }; // success token
        }
    },
    {
        "sys_get_input",
        [](const std::vector<pulsar::interpreter::Token>& args) {
            if (!args.empty()) {
                throw std::runtime_error("system_get_input: Expected no arguments, got " + std::to_string(args.size()) + 
                                       ". Usage: system_get_input");
            }
            
            std::string input = PULSARSTD_api::system::get_input();
            return pulsar::interpreter::Token{ input, pulsar::interpreter::TokenType::STRING };
        }
    }
};

std::unordered_map<std::string, std::function<pulsar::interpreter::Token(const std::vector<pulsar::interpreter::Token>&)>>& pulsar::interpreter::getStandardLibraryFunctions() {
    return standardLibraryFunctions;
}