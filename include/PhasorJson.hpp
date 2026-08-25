/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                                                             //
//   PPPPPPP  H     H      AA      SSSSSSS  OOOOOOO  RRRRRRR    L            AA      NN    N  GGGGGGG  U     U      AA      GGGGGGG  EEEEEEE   //
//   P     P  H     H     A  A     S        O     O  R     R    L           A  A     N N   N  G        U     U     A  A     G        E         //
//   PPPPPPP  HHHHHHH    AAAAAA    SSSSSSS  O     O  RRRRRRR    L          AAAAAA    N  N  N  G  GGGG  U     U    AAAAAA    G  GGGG  EEEEEEE   //
//   P        H     H   A      A         S  O     O  R    R     L         A      A   N   N N  G     G  U     U   A      A   G     G  E         //
//   P        H     H  A        A  SSSSSSS  OOOOOOO  R     R    LLLLLLL  A        A  N    NN  GGGGGGG  UUUUUUU  A        A  GGGGGGG  EEEEEEE   //
//                                                                                                                                             //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Copyright 2026 Daniel McGuire
// Phasor Toolchain Licensed under the Apache License, Version 2.0 (the "License");
// Phasor Runtime Licensed under the Apache License (with LLVM-Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://llvm.org/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include "PhasorString.hpp"

namespace Phasor {

class Value;

namespace PhsJson {
    using json_iterator = std::string_view::const_iterator;

    inline void skip_whitespace(json_iterator& it, json_iterator end) {
        while (it != end && std::isspace(static_cast<unsigned char>(*it))) ++it;
    }

    inline PhsString parse_json_string(json_iterator& it, json_iterator end) {
        if (it == end || *it != '"')
            throw std::runtime_error("Expected '\"'");
        ++it;

        PhsString result;
        while (it != end && *it != '"') {
            if (*it == '\\') {
                ++it;
                if (it == end) throw std::runtime_error("Unexpected end of string");
                switch (*it) {
                    case '"':  result += '"';  break;
                    case '\\': result += '\\'; break;
                    case '/':  result += '/';  break;
                    case 'b':  result += '\b'; break;
                    case 'f':  result += '\f'; break;
                    case 'n':  result += '\n'; break;
                    case 'r':  result += '\r'; break;
                    case 't':  result += '\t'; break;
                    case 'u': {
                        if (std::distance(it, end) < 5)
                            throw std::runtime_error("Invalid \\u escape");
                        ++it; // move past 'u'
                        char hex[5] = {0};
                        for (int i = 0; i < 4; ++i, ++it) {
                            if (it == end || !std::isxdigit(static_cast<unsigned char>(*it)))
                                throw std::runtime_error("Invalid hex digit in \\u");
                            hex[i] = *it;
                        }
                        --it;
                        unsigned long codepoint = std::strtoul(hex, nullptr, 16);
                        if (codepoint < 0x80) {
                            result += static_cast<char>(codepoint);
                        } else if (codepoint < 0x800) {
                            result += static_cast<char>(0xC0 | (codepoint >> 6));
                            result += static_cast<char>(0x80 | (codepoint & 0x3F));
                        } else if (codepoint < 0xD800 || codepoint > 0xDFFF) {
                            result += static_cast<char>(0xE0 | (codepoint >> 12));
                            result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                            result += static_cast<char>(0x80 | (codepoint & 0x3F));
                        } else {
                            result += "\xEF\xBF\xBD";
                        }
                        break;
                    }
                    default:
                        throw std::runtime_error("Invalid escape character");
                }
            } else {
                result += *it;
            }
            ++it;
        }
        if (it == end) throw std::runtime_error("Unterminated string");
        ++it;
        return result;
    }

    inline Value parse_value(json_iterator& it, json_iterator end);

    inline Value parse_json_number(json_iterator& it, json_iterator end) {
        auto start = it;
        bool is_float = false;

        if (it != end && *it == '-') ++it;
        if (it == end || !std::isdigit(static_cast<unsigned char>(*it)))
            throw std::runtime_error("Invalid number");
        while (it != end && std::isdigit(static_cast<unsigned char>(*it))) ++it;

        if (it != end && *it == '.') {
            is_float = true;
            ++it;
            if (it == end || !std::isdigit(static_cast<unsigned char>(*it)))
                throw std::runtime_error("Invalid number");
            while (it != end && std::isdigit(static_cast<unsigned char>(*it))) ++it;
        }
        if (it != end && (*it == 'e' || *it == 'E')) {
            is_float = true;
            ++it;
            if (it != end && (*it == '+' || *it == '-')) ++it;
            if (it == end || !std::isdigit(static_cast<unsigned char>(*it)))
                throw std::runtime_error("Invalid number");
            while (it != end && std::isdigit(static_cast<unsigned char>(*it))) ++it;
        }

        PhsString number_str{std::string_view(start, it)};
        if (is_float) {
            try {
                return Value(std::stod(number_str));
            } catch (...) {
                throw std::runtime_error("Number out of range");
            }
        } else {
            try {
                i64 val = std::stoll(number_str);
                return Value(val);
            } catch (const std::out_of_range&) {
                try {
                    return Value(std::stod(number_str));
                } catch (...) {
                    throw std::runtime_error("Number out of range");
                }
            }
        }
    }

    inline Value parse_json_array(json_iterator& it, json_iterator end) {
        if (it == end || *it != '[')
            throw std::runtime_error("Expected '['");
        ++it;
        Value::ArrayInstance elements;
        skip_whitespace(it, end);
        if (it != end && *it != ']') {
            while (true) {
                elements.push_back(parse_value(it, end));
                skip_whitespace(it, end);
                if (it != end && *it == ',') {
                    ++it;
                    skip_whitespace(it, end);
                } else {
                    break;
                }
            }
        }
        if (it == end || *it != ']')
            throw std::runtime_error("Expected ']'");
        ++it;
        return Value(std::make_shared<Value::ArrayInstance>(std::move(elements)));
    }

    inline Value parse_json_object(json_iterator& it, json_iterator end) {
        if (it == end || *it != '{')
            throw std::runtime_error("Expected '{'");
        ++it;
        auto struct_ptr = std::make_shared<Value::StructInstance>();
        struct_ptr->structName = PhsString();

        skip_whitespace(it, end);
        if (it != end && *it != '}') {
            while (true) {
                skip_whitespace(it, end);
                PhsString key = parse_json_string(it, end);
                skip_whitespace(it, end);
                if (it == end || *it != ':')
                    throw std::runtime_error("Expected ':'");
                ++it;
                Value val = parse_value(it, end);
                struct_ptr->fields[key] = std::move(val);
                skip_whitespace(it, end);
                if (it != end && *it == ',') {
                    ++it;
                    skip_whitespace(it, end);
                } else {
                    break;
                }
            }
        }
        if (it == end || *it != '}')
            throw std::runtime_error("Expected '}'");
        ++it;
        return Value(std::move(struct_ptr));
    }

    inline Value parse_value(json_iterator& it, json_iterator end) {
        skip_whitespace(it, end);
        if (it == end)
            throw std::runtime_error("Unexpected end of input");

        if (*it == '"') {
            return Value(parse_json_string(it, end));
        } else if (*it == '[') {
            return parse_json_array(it, end);
        } else if (*it == '{') {
            return parse_json_object(it, end);
        } else if (*it == 't' && std::string_view(it, end).substr(0, 4) == "true") {
            it += 4;
            return Value(true);
        } else if (*it == 'f' && std::string_view(it, end).substr(0, 5) == "false") {
            it += 5;
            return Value(false);
        } else if (*it == 'n' && std::string_view(it, end).substr(0, 4) == "null") {
            it += 4;
            return Value();
        } else if (*it == '-' || std::isdigit(static_cast<unsigned char>(*it))) {
            return parse_json_number(it, end);
        } else {
            throw std::runtime_error("Unexpected character");
        }
    }
} // namespace json

}
