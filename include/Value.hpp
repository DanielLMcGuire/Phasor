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
	using DataType = std::variant<std::monostate, bool, int64_t, double, std::string, std::shared_ptr<StructInstance>,
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
	Value(const std::string &s) : data(s)
	{
	}
	/// @brief String constructor
	Value(const char *s) : data(std::string(s))
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
	ValueType getType() const
	{
		if (std::holds_alternative<std::monostate>(data))
			return ValueType::Null;
		if (std::holds_alternative<bool>(data))
			return ValueType::Bool;
		if (std::holds_alternative<int64_t>(data))
			return ValueType::Int;
		if (std::holds_alternative<double>(data))
			return ValueType::Float;
		if (std::holds_alternative<std::string>(data))
			return ValueType::String;
		if (std::holds_alternative<std::shared_ptr<StructInstance>>(data))
			return ValueType::Struct;
		if (std::holds_alternative<std::shared_ptr<ArrayInstance>>(data))
			return ValueType::Array;
		return ValueType::Null; // Should not be reached if default constructed
	}

	inline static Value typeToString(const ValueType &type)
	{
		switch (type)
		{
		case ValueType::Null:
			return Value("null");
		case ValueType::Bool:
			return Value("bool");
		case ValueType::Int:
			return Value("int");
		case ValueType::Float:
			return Value("float");
		case ValueType::String:
			return Value("string");
		case ValueType::Struct:
			return Value("struct");
		case ValueType::Array:
			return Value("array");
		default:
			return Value("unknown");
		}
	}

	/// @brief Check if the value is null
	bool isNull() const
	{
		return getType() == ValueType::Null;
	}
	/// @brief Check if the value is a boolean
	bool isBool() const
	{
		return getType() == ValueType::Bool;
	}
	/// @brief Check if the value is an integer
	bool isInt() const
	{
		return getType() == ValueType::Int;
	}
	/// @brief Check if the value is a double
	bool isFloat() const
	{
		return getType() == ValueType::Float;
	}
	/// @brief Check if the value is a string
	bool isString() const
	{
		return getType() == ValueType::String;
	}
	/// @brief Check if the value is a number
	bool isNumber() const
	{
		return isInt() || isFloat();
	}
	/// @brief Check if the value is an array
	bool isArray() const
	{
		return std::holds_alternative<std::shared_ptr<ArrayInstance>>(data);
	}

	/// @brief Get the value as a boolean
	bool asBool() const
	{
		return std::get<bool>(data);
	}
	/// @brief Get the value as an integer
	int64_t asInt() const
	{
		if (isInt())
			return std::get<int64_t>(data);
		if (isFloat())
			return static_cast<int64_t>(std::get<double>(data));
		return 0;
	}
	/// @brief Get the value as a double
	double asFloat() const
	{
		if (isFloat())
			return std::get<double>(data);
		if (isInt())
			return static_cast<double>(std::get<int64_t>(data));
		return 0.0;
	}
	/// @brief Get the value as a string
	std::string asString() const
	{
		if (isString())
			return std::get<std::string>(data);
		return toString();
	}
	/// @brief Get the value as an array
	std::shared_ptr<ArrayInstance> asArray()
	{
		return std::get<std::shared_ptr<ArrayInstance>>(data);
	}

	/// @brief Get the value as an array (const)
	const std::shared_ptr<const ArrayInstance> asArray() const
	{
		return std::get<std::shared_ptr<ArrayInstance>>(data);
	}

	/// @brief Add two values
	Value operator+(const Value &other) const
	{
		if (isInt() && other.isInt())
			return Value(asInt() + other.asInt());
		if (isNumber() && other.isNumber())
			return Value(asFloat() + other.asFloat());
		if (isString() && other.isString())
			return Value(asString() + other.asString());
		throw std::runtime_error("Cannot add these value types");
	}

	/// @brief Subtract two values
	Value operator-(const Value &other) const
	{
		if (isInt() && other.isInt())
			return Value(asInt() - other.asInt());
		if (isNumber() && other.isNumber())
			return Value(asFloat() - other.asFloat());
		throw std::runtime_error("Cannot subtract these value types");
	}

	/// @brief Multiply two values
	Value operator*(const Value &other) const
	{
		if (isInt() && other.isInt())
			return Value(asInt() * other.asInt());
		if (isNumber() && other.isNumber())
			return Value(asFloat() * other.asFloat());
		throw std::runtime_error("Cannot multiply these value types");
	}

	/// @brief Divide two values
	Value operator/(const Value &other) const
	{
		if (isInt() && other.isInt())
		{
			if (other.asInt() == 0)
				throw std::runtime_error("Division by zero");
			return Value(asInt() / other.asInt());
		}
		if (isNumber() && other.isNumber())
		{
			if (other.asFloat() == 0.0)
				throw std::runtime_error("Division by zero");
			return Value(asFloat() / other.asFloat());
		}
		throw std::runtime_error("Cannot divide these value types");
	}

	/// @brief Modulo two values
	Value operator%(const Value &other) const
	{
		if (isInt() && other.isInt())
		{
			if (other.asInt() == 0)
				throw std::runtime_error("Modulo by zero");
			return Value(asInt() % other.asInt());
		}
		throw std::runtime_error("Modulo requires integer operands");
	}

	/// @brief Unary negation
	Value operator-() const
	{
		if (isInt())
			return Value(-asInt());
		if (isFloat())
			return Value(-asFloat());
		throw std::runtime_error("Cannot negate this value type");
	}

	/// @brief Logical negation
	Value operator!() const
	{
		return Value(!isTruthy());
	}

	/// @brief Logical AND
	Value logicalAnd(const Value &other) const
	{
		return Value(isTruthy() && other.isTruthy());
	}

	/// @brief Logical OR
	Value logicalOr(const Value &other) const
	{
		return Value(isTruthy() || other.isTruthy());
	}

	/// @brief Helper to determine truthiness
	bool isTruthy() const
	{
		if (isNull())
			return false;
		if (isBool())
			return asBool();
		if (isInt())
			return asInt() != 0;
		if (isFloat())
			return asFloat() != 0.0;
		if (isString())
		{
			if (asString() == "true" || asString() == "1")
				return true;
			else if (asString() == "false" || asString() == "0")
				return false;
			return !asString().empty();
		}
		return false;
	}

	/// @brief Comparison operations
	bool operator==(const Value &other) const
	{
		if (getType() != other.getType())
			return false;
		if (isNull())
			return true;
		if (isBool())
			return asBool() == other.asBool();
		if (isInt())
			return asInt() == other.asInt();
		if (isFloat())
			return asFloat() == other.asFloat();
		if (isString())
			return asString() == other.asString();
		if (isArray())
		{
			if (!other.isArray())
				return false;
			const auto &self_arr = *asArray();
			const auto &other_arr = *other.asArray();
			return self_arr == other_arr;
		}
		return false;
	}

	/// @brief Inequality comparison
	bool operator!=(const Value &other) const
	{
		return !(*this == other);
	}

	/// @brief Less than comparison
	bool operator<(const Value &other) const
	{
		if (isInt() && other.isInt())
			return asInt() < other.asInt();
		if (isNumber() && other.isNumber())
			return asFloat() < other.asFloat();
		if (isString() && other.isString())
			return asString() < other.asString();
		throw std::runtime_error("Cannot compare these value types ");
	}

	/// @brief Greater than comparison
	bool operator>(const Value &other) const
	{
		if (isInt() && other.isInt())
			return asInt() > other.asInt();
		if (isNumber() && other.isNumber())
			return asFloat() > other.asFloat();
		if (isString() && other.isString())
			return asString() > other.asString();
		throw std::runtime_error("Cannot compare these value types ");
	}

	/// @brief Less than or equal to comparison
	bool operator<=(const Value &other) const
	{
		return !(*this > other);
	}
	/// @brief Greater than or equal to comparison
	bool operator>=(const Value &other) const
	{
		return !(*this < other);
	}

	/// @brief Convert to string for printing
	std::string toString() const
	{
		if (isNull())
			return "null";
		if (isBool())
			return asBool() ? "true" : "false";
		if (isInt())
			return std::to_string(asInt());
		if (isFloat())
			return std::to_string(asFloat());
		if (isString())
			return asString();
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
	const char *c_str() const
	{
		if (!isString())
			throw std::runtime_error("c_str() can only be called on string values");
		return std::get<std::string>(data).c_str();
	}

	/// @brief Print to output stream
	friend std::ostream &operator<<(std::ostream &os, const Value &v)
	{
		os << v.toString();
		return os;
	}

	bool isStruct() const
	{
		return std::holds_alternative<std::shared_ptr<StructInstance>>(data);
	}

	std::shared_ptr<StructInstance> asStruct()
	{
		return std::get<std::shared_ptr<StructInstance>>(data);
	}

	const std::shared_ptr<const StructInstance> asStruct() const
	{
		return std::get<std::shared_ptr<StructInstance>>(data);
	}

	static Value createStruct(const std::string &name)
	{
		return Value(std::make_shared<StructInstance>(StructInstance{name}));
	}

	static Value createArray(std::vector<Value> elements = {})
	{
		return Value(std::make_shared<ArrayInstance>(std::move(elements)));
	}

	Value getField(const std::string &name) const
	{
		if (!std::holds_alternative<std::shared_ptr<StructInstance>>(data))
			throw std::runtime_error("getField() called on non-struct value");
		auto s = std::get<std::shared_ptr<StructInstance>>(data);
		auto it = s->fields.find(name);
		if (it == s->fields.end())
			return Value();
		return it->second;
	}

	void setField(const std::string &name, Value value)
	{
		if (!std::holds_alternative<std::shared_ptr<StructInstance>>(data))
			throw std::runtime_error("setField() called on non-struct value");
		auto s = std::get<std::shared_ptr<StructInstance>>(data);
		s->fields[name] = std::move(value);
	}

	bool hasField(const std::string &name) const
	{
		if (!std::holds_alternative<std::shared_ptr<StructInstance>>(data))
			return false;
		auto s = std::get<std::shared_ptr<StructInstance>>(data);
		return s->fields.find(name) != s->fields.end();
	}
};
} // namespace Phasor

template <>
struct std::formatter<Phasor::Value>
{
    enum class Style { Value, TypeOnly, TypeValue, Debug, Quoted };
    Style       style    = Style::Value;
    std::string passthrough;

    constexpr auto parse(std::format_parse_context &ctx)
    {
        auto it  = ctx.begin();
        auto end = ctx.end();

        std::string_view full(it, end);
        if (auto close = full.find('}'); close != std::string_view::npos)
            full = full.substr(0, close);

        std::string_view inner = full;
        if (!full.empty())
        {
            switch (full.back())
            {
            case 't': style = Style::TypeOnly;  inner = full.substr(0, full.size() - 1); break;
            case 'T': style = Style::TypeValue; inner = full.substr(0, full.size() - 1); break;
            case '?': style = Style::Debug;     inner = full.substr(0, full.size() - 1); break;
            case 'q': style = Style::Quoted;    inner = full.substr(0, full.size() - 1); break;
            default:  break;
            }
        }

        passthrough = std::string(inner);
        return it + full.size();
    }

    template <typename FormatContext>
    auto format(const Phasor::Value &v, FormatContext &ctx) const
    {
        std::string fmtstr = "{:" + passthrough + "}";

        auto fwd = [&]<typename T>(const T &val) {
            return std::vformat_to(ctx.out(), fmtstr, std::make_format_args(val));
        };

        using namespace Phasor;

        switch (style)
        {
        case Style::TypeOnly:
            return fwd(Value::typeToString(v.getType()).asString());

        case Style::TypeValue:
            return fwd(Value::typeToString(v.getType()).asString()
                       + "(" + v.toString() + ")");

        case Style::Debug:
            return fwd(debug_repr(v));

        case Style::Quoted:
            if (v.isString())
                return fwd("\"" + escape(v.asString()) + "\"");
            [[fallthrough]];

        case Style::Value:
        default:
            switch (v.getType())
            {
            case ValueType::Null:   return std::format_to(ctx.out(), "null");
            case ValueType::Bool:   return fwd(v.asBool());
            case ValueType::Int:    return fwd(v.asInt());    // int64_t — gets {:08d}, {:x}, etc.
            case ValueType::Float:  return fwd(v.asFloat());  // double  — gets {:.2f}, {:e}, etc.
            case ValueType::String: return fwd(v.asString()); // string  — gets fill/align/width
            case ValueType::Array:  return fwd(v.toString());
            case ValueType::Struct: return fwd(v.toString());
            }
        }
        return ctx.out(); // unreachable
    }

  private:
    static std::string debug_repr(const Phasor::Value &v)
    {
        using Phasor::ValueType;
        switch (v.getType())
        {
        case ValueType::Null:   return "null";
        case ValueType::String: return "\"" + escape(v.asString()) + "\"";
        case ValueType::Array:
        {
            const auto &arr = *v.asArray();
            std::string out = "[";
            for (std::size_t i = 0; i < arr.size(); ++i)
            {
                out += debug_repr(arr[i]);
                if (i + 1 < arr.size()) out += ", ";
            }
            return out + "]";
        }
        case ValueType::Struct:
        {
            const auto &s = *v.asStruct();
            std::string out = s.structName + " { ";
            bool first = true;
            for (const auto &[k, val] : s.fields)
            {
                if (!first) out += ", ";
                out += k + ": " + debug_repr(val);
                first = false;
            }
            return out + " }";
        }
        default: return v.toString();
        }
    }

    static std::string escape(const std::string &s)
    {
        std::string out;
        out.reserve(s.size());
        for (char c : s)
            switch (c)
            {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;
            }
        return out;
    }
};