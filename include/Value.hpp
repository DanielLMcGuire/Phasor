// Copyright 2026 Daniel McGuire
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// README
//
// Provides types for the Phasor (and Pulsar) Programming Language.
// Wraps a std::variant over null, bool, int64_t, double, string, struct, and array,
// with structs, arrays, and strings heap-allocated via std::shared_ptr. Provides arithmetic,
// comparison, and logical operators, and isTruthy() and toString().
//
// Also includes a std::formatter<Phasor::Value> implementation for use with std::format (or std::print).
// Supports four format specifiers: default (value as-is), t (type name only),
// T (type and value), ? (debug repr with quoted strings and recursive expansion), and
// q (quoted strings, default otherwise).

#pragma once
#include <iostream>
#include <string>
#include <variant>
#include <unordered_map>
#include <memory>
#include <vector>
#include <format>
#include <string>

/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/**
 * @brief Runtime value types for the VM
 */
enum class ValueType
{
	Null,
	Bool,
	Int,
	Float,
	String,
	Struct,
	Array
};

/**
 * @brief A value in the Phasor VM
 *
 * Uses std::variant for type-safe union
 */
class Value
{
  public:
	struct StructInstance
	{
		std::string                            structName;
		std::unordered_map<std::string, Value> fields;
	};
	using ArrayInstance = std::vector<Value>;

  private:
	using DataType = std::variant<std::monostate, bool, int64_t, double, std::shared_ptr<std::string>,
	                              std::shared_ptr<StructInstance>,
	                              std::shared_ptr<ArrayInstance>>;

	DataType data;

  public:
	/// @brief Default constructor
	Value() : data(std::monostate{})
	{
	}
	/// @brief Boolean constructor
	Value(bool b) : data(b)
	{
	}
	/// @brief Integer constructor
	Value(int64_t i) : data(i)
	{
	}
	/// @brief Integer constructor
	Value(int i) : data(static_cast<int64_t>(i))
	{
	}
	/// @brief Double constructor
	Value(double d) : data(d)
	{
	}
	/// @brief String constructor
	Value(const std::string &s) : data(std::make_shared<std::string>(s))
	{
	}
	/// @brief String constructor
	Value(const char *s) : data(std::make_shared<std::string>(s))
	{
	}
	/// @brief Struct constructor
	Value(std::shared_ptr<StructInstance> s) : data(std::move(s))
	{
	}
	/// @brief Array constructor
	Value(std::shared_ptr<ArrayInstance> a) : data(std::move(a))
	{
	}

	/// @brief Get the type of the value
	[[nodiscard]] ValueType getType() const noexcept {
		return static_cast<ValueType>(data.index());
	}

	static Value typeToString(const ValueType &type)
	{
		switch (type)
		{
		case ValueType::Null:
			return {"null"};
		case ValueType::Bool:
			return {"bool"};
		case ValueType::Int:
			return {"int"};
		case ValueType::Float:
			return {"float"};
		case ValueType::String:
			return {"string"};
		case ValueType::Struct:
			return {"struct"};
		case ValueType::Array:
			return {"array"};
		default:
			return {"unknown"};
		}
	}

	/// @brief Check if the value is null
	[[nodiscard]] bool isNull()   const noexcept { return data.index() == 0; }
	[[nodiscard]] bool isBool()   const noexcept { return data.index() == 1; }
	[[nodiscard]] bool isInt()    const noexcept { return data.index() == 2; }
	[[nodiscard]] bool isFloat()  const noexcept { return data.index() == 3; }
	[[nodiscard]] bool isString() const noexcept { return data.index() == 4; }
	[[nodiscard]] bool isNumber() const noexcept { return data.index() == 2 || data.index() == 3; }
	/// @brief Check if the value is an array
	[[nodiscard]] bool isArray() const noexcept
	{
		return std::holds_alternative<std::shared_ptr<ArrayInstance>>(data);
	}

	/// @brief Get the value as a boolean
	[[nodiscard]] bool asBool() const noexcept
	{
		return std::get<bool>(data);
	}
	/// @brief Get the value as an integer
	[[nodiscard]] int64_t asInt() const noexcept
	{
		if (isInt())
		{
			return std::get<int64_t>(data);
		}
		if (isFloat())
		{
			return static_cast<int64_t>(std::get<double>(data));
		}
		return 0;
	}
	/// @brief Get the value as a double
	[[nodiscard]] double asFloat() const noexcept
	{
		if (isFloat())
		{
			return std::get<double>(data);
		}
		if (isInt())
		{
			return static_cast<double>(std::get<int64_t>(data));
		}
		return 0.0;
	}
	/// @brief Get the value as a string
	[[nodiscard]] std::string asString() const noexcept
	{
		if (isString())
		{
			return *std::get<std::shared_ptr<std::string>>(data);
		}
		return toString();
	}
	/// @brief Get the value as an array
	std::shared_ptr<ArrayInstance> asArray()
	{
		return std::get<std::shared_ptr<ArrayInstance>>(data);
	}

	/// @brief Get the value as an array (const)
	[[nodiscard]] std::shared_ptr<const ArrayInstance> asArray() const noexcept
	{
		return std::get<std::shared_ptr<ArrayInstance>>(data);
	}

	/// @brief Add two values
	Value operator+(const Value &other) const
	{
		if (isInt() && other.isInt())
		{
			return {asInt() + other.asInt()};
		}
		if (isNumber() && other.isNumber())
		{
			return {asFloat() + other.asFloat()};
		}
		if (isString() && other.isString())
		{
			return Value(asString() + other.asString());
		}
		throw std::runtime_error("Cannot add these value types");
	}

	/// @brief Subtract two values
	Value operator-(const Value &other) const
	{
		if (isInt() && other.isInt())
		{
			return {asInt() - other.asInt()};
		}
		if (isNumber() && other.isNumber())
		{
			return {asFloat() - other.asFloat()};
		}
		throw std::runtime_error("Cannot subtract these value types");
	}

	Value &operator--()
	{
		if (isInt())
		{
			data = asInt() - 1;
			return *this;
		}
		if (isFloat())
		{
			data = asFloat() - 1.0;
			return *this;
		}
		throw std::runtime_error("Cannot decrement this value type");
	}

	Value &operator++()
	{
		if (isInt())
		{
			data = asInt() + 1;
			return *this;
		}
		if (isFloat())
		{
			data = asFloat() + 1.0;
			return *this;
		}
		throw std::runtime_error("Cannot increment this value type");
	}

	/// @brief Multiply two values
	Value operator*(const Value &other) const
	{
		if (isInt() && other.isInt())
		{
			return {asInt() * other.asInt()};
		}
		if (isNumber() && other.isNumber())
		{
			return {asFloat() * other.asFloat()};
		}
		throw std::runtime_error("Cannot multiply these value types");
	}

	/// @brief Divide two values
	Value operator/(const Value &other) const
	{
		if (isInt() && other.isInt())
		{
			if (other.asInt() == 0)
			{
				throw std::runtime_error("Division by zero");
			}
			return {asInt() / other.asInt()};
		}
		if (isNumber() && other.isNumber())
		{
			if (other.asFloat() == 0.0)
			{
				throw std::runtime_error("Division by zero");
			}
			return {asFloat() / other.asFloat()};
		}
		throw std::runtime_error("Cannot divide these value types");
	}

	/// @brief Modulo two values
	Value operator%(const Value &other) const
	{
		if (isInt() && other.isInt())
		{
			if (other.asInt() == 0)
			{
				throw std::runtime_error("Modulo by zero");
			}
			return {asInt() % other.asInt()};
		}
		throw std::runtime_error("Modulo requires integer operands");
	}

	/// @brief Logical negation
	Value operator!() const noexcept
	{
		return {!isTruthy()};
	}

	/// @brief Logical AND
	[[nodiscard]] Value logicalAnd(const Value &other) const noexcept
	{
		return {isTruthy() && other.isTruthy()};
	}

	/// @brief Logical OR
	[[nodiscard]] Value logicalOr(const Value &other) const noexcept
	{
		return {isTruthy() || other.isTruthy()};
	}

	/// @brief Helper to determine truthiness
	[[nodiscard]] bool isTruthy() const noexcept
	{
		if (isNull())
		{
			return false;
		}
		if (isBool())
		{
			return asBool();
		}
		if (isInt())
		{
			return asInt() != 0;
		}
		if (isFloat())
		{
			return asFloat() != 0.0;
		}
		if (isString())
		{
			return !asString().empty();
		}
		return false;
	}

	/// @brief Comparison operations
	bool operator==(const Value &other) const noexcept
	{
		if (getType() != other.getType())
		{
			return false;
		}
		if (isNull())
		{
			return true;
		}
		if (isBool())
		{
			return asBool() == other.asBool();
		}
		if (isInt())
		{
			return asInt() == other.asInt();
		}
		if (isFloat())
		{
			return asFloat() == other.asFloat();
		}
		if (isString())
		{
			return asString() == other.asString();
		}
		if (isArray())
		{
			if (!other.isArray())
			{
				return false;
			}
			const auto &self_arr = *asArray();
			const auto &other_arr = *other.asArray();
			return self_arr == other_arr;
		}
		return false;
	}

	/// @brief Inequality comparison
	bool operator!=(const Value &other) const noexcept
	{
		return !(*this == other);
	}

	/// @brief Less than comparison
	bool operator<(const Value &other) const
	{
		if (isInt() && other.isInt())
		{
			return asInt() < other.asInt();
		}
		if (isNumber() && other.isNumber())
		{
			return asFloat() < other.asFloat();
		}
		if (isString() && other.isString())
		{
			return asString() < other.asString();
		}
		throw std::runtime_error("Cannot compare these value types ");
	}

	/// @brief Greater than comparison
	bool operator>(const Value &other) const
	{
		if (isInt() && other.isInt())
		{
			return asInt() > other.asInt();
		}
		if (isNumber() && other.isNumber())
		{
			return asFloat() > other.asFloat();
		}
		if (isString() && other.isString())
		{
			return asString() > other.asString();
		}
		throw std::runtime_error("Cannot compare these value types ");
	}

	/// @brief Less than or equal to comparison
	bool operator<=(const Value &other) const noexcept
	{
		return !(*this > other);
	}
	/// @brief Greater than or equal to comparison
	bool operator>=(const Value &other) const noexcept
	{
		return !(*this < other);
	}

	/// @brief Convert to string for printing
	[[nodiscard]] std::string toString() const noexcept
	{
		if (isNull())
		{
			return "null";
		}
		if (isBool())
		{
			return asBool() ? "true" : "false";
		}
		if (isInt())
		{
			return std::to_string(asInt());
		}
		if (isFloat())
		{
			return std::to_string(asFloat());
		}
		if (isString())
		{
			[[likely]] return asString();
		}
		if (isArray())
		{
			std::string result = "[";
			const auto &arr = *asArray();
			for (size_t i = 0; i < arr.size(); ++i)
			{
				result += arr[i].toString();
				if (i < arr.size() - 1)
				{
					result += ", ";
				}
			}
			result += "]";
			return result;
		}
		return "unknown";
	}

	/// @brief Convert to C Style String
	[[nodiscard]] const char *c_str() const
	{
		if (!isString())
		{
			[[unlikely]] throw std::runtime_error("c_str() can only be called on string values");
		}
		return std::get<std::shared_ptr<std::string>>(data)->c_str();
	}

	/// @brief Print to output stream
	friend std::ostream &operator<<(std::ostream &os, const Value &v)
	{
		os << v.toString();
		return os;
	}

	[[nodiscard]] bool isStruct() const
	{
		return std::holds_alternative<std::shared_ptr<StructInstance>>(data);
	}

	std::shared_ptr<StructInstance> asStruct()
	{
		return std::get<std::shared_ptr<StructInstance>>(data);
	}

	[[nodiscard]] std::shared_ptr<const StructInstance> asStruct() const noexcept
	{
		return std::get<std::shared_ptr<StructInstance>>(data);
	}

	static Value createStruct(const std::string &name)
	{
		return Value(std::make_shared<StructInstance>(StructInstance{.structName = name, .fields = {}}));
	}

	static Value createArray(std::vector<Value> elements = {})
	{
		return {std::make_shared<ArrayInstance>(std::move(elements))};
	}

	[[nodiscard]] Value getField(const std::string &name) const
	{
		if (!std::holds_alternative<std::shared_ptr<StructInstance>>(data))
		{
			[[unlikely]] throw std::runtime_error("getField() called on non-struct value");
		}
		auto s = std::get<std::shared_ptr<StructInstance>>(data);
		auto it = s->fields.find(name);
		if (it == s->fields.end())
		{
			return {};
		}
		return it->second;
	}

	void setField(const std::string &name, Value value)
	{
		if (!std::holds_alternative<std::shared_ptr<StructInstance>>(data))
		{
			[[unlikely]] throw std::runtime_error("setField() called on non-struct value");
		}
		auto s = std::get<std::shared_ptr<StructInstance>>(data);
		s->fields[name] = std::move(value);
	}

	[[nodiscard]] bool hasField(const std::string &name) const noexcept
	{
		if (!std::holds_alternative<std::shared_ptr<StructInstance>>(data))
		{
			return false;
		}
		auto s = std::get<std::shared_ptr<StructInstance>>(data);
		return s->fields.contains(name);
	}
};
} // namespace Phasor

template <> struct std::formatter<Phasor::Value>
{
	enum class Style
	{
		Value,
		TypeOnly,
		TypeValue,
		Debug,
		Quoted
	};
	Style            style = Style::Value;
	std::string_view passthrough;

	constexpr auto parse(std::format_parse_context &ctx)
	{
		auto it = ctx.begin();
		auto end = ctx.end();

		auto close = it;
		while (close != end && *close != '}')
		{
			++close;
		}

		std::string_view full(&*it, static_cast<size_t>(close - it));
		std::string_view inner = full;

		if (!full.empty())
		{
			switch (full.back())
			{
			case 't':
				style = Style::TypeOnly;
				inner = full.substr(0, full.size() - 1);
				break;
			case 'T':
				style = Style::TypeValue;
				inner = full.substr(0, full.size() - 1);
				break;
			case '?':
				style = Style::Debug;
				inner = full.substr(0, full.size() - 1);
				break;
			case 'q':
				style = Style::Quoted;
				inner = full.substr(0, full.size() - 1);
				break;
			default:
				break;
			}
		}

		passthrough = inner;
		return close;
	}

	template <typename FormatContext> auto format(const Phasor::Value &v, FormatContext &ctx) const
	{
		std::string fmtstr;
		fmtstr.reserve(passthrough.size() + 3);
		fmtstr += "{:";
		fmtstr += passthrough;
		fmtstr += '}';

		auto fwd = [&]<typename T>(const T &val) {
			return std::vformat_to(ctx.out(), fmtstr, std::make_format_args(val));
		};

		using namespace Phasor;

		switch (style)
		{
		case Style::TypeOnly:
			return fwd(Value::typeToString(v.getType()).asString());

		case Style::TypeValue:
			return fwd(Value::typeToString(v.getType()).asString() + "(" + escapeString(v.toString()) + ")");

		case Style::Debug:
			return fwd(debug_repr(v));

		case Style::Quoted:
			if (v.isString())
			{
				return fwd("\"" + escapeString(v.asString()) + "\"");
			}
			[[fallthrough]];

		case Style::Value:
		default:
			switch (v.getType())
			{
			case ValueType::Null:
				return std::format_to(ctx.out(), "null");
			case ValueType::Bool:
				return fwd(v.asBool());
			case ValueType::Int:
				return fwd(v.asInt());
			case ValueType::Float:
				return fwd(v.asFloat());
			case ValueType::String:
				return fwd(debug_repr(escapeString(v.asString())));
			case ValueType::Array:
				return fwd(v.toString());
			case ValueType::Struct:
				return fwd(v.toString());
			}
		}
		return ctx.out();
	}

  private:
	static std::string escapeString(const std::string &input)
	{
		std::string output;
		output.reserve(input.size());
		for (char c : input)
		{
			switch (c)
			{
			case '\n':
				output += "\\n";
				break;
			case '\t':
				output += "\\t";
				break;
			case '\r':
				output += "\\r";
				break;
			case '\0':
				output += "\\0";
				break;
			case '\\':
				output += "\\\\";
				break;
			case '\"':
				output += "\\\"";
				break;
			case '\'':
				output += "\\'";
				break;
			case '\a':
				output += "\\a";
				break;
			case '\b':
				output += "\\b";
				break;
			case '\f':
				output += "\\f";
				break;
			case '\v':
				output += "\\v";
				break;
			default:
				if (c < 0x20 || c == 0x7F)
				{
					char buf[5];
					snprintf(buf, sizeof(buf), "\\x%02X", (unsigned char)c);
					output += buf;
				}
				else
				{
					output += c;
				}
				break;
			}
		}
		return output;
	}

	static std::string debug_repr(const Phasor::Value &v)
	{
		using Phasor::ValueType;
		switch (v.getType())
		{
		case ValueType::Null:
			return "null";
		case ValueType::String:
			return "\"" + escapeString(v.asString()) + "\"";
		case ValueType::Array: {
			const auto &arr = *v.asArray();
			std::string out = "[";
			for (std::size_t i = 0; i < arr.size(); ++i)
			{
				out += debug_repr(arr[i]);
				if (i + 1 < arr.size())
				{
					out += ", ";
				}
			}
			return out + "]";
		}
		case ValueType::Struct: {
			const auto &s = *v.asStruct();
			std::string out = s.structName + " { ";
			bool        first = true;
			for (const auto &[k, val] : s.fields)
			{
				if (!first)
				{
					out += ", ";
				}
				out += k + ": " + debug_repr(val);
				first = false;
			}
			return out + " }";
		}
		default:
			return v.toString();
		}
	}
};