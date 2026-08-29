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
// Phasor Runtime Licensed under the Apache License (with Phasor Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// http://www.apache.org/licenses/LICENSE-2.0
// or https://phasor.pages.dev/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// README
//
// Provides types for the Phasor Programming Language.
// Wraps a std::variant over null, bool, int64_t, double, string, struct, and array,
// with structs, arrays, and strings heap-allocated via std::shared_ptr. Provides arithmetic,
// comparison, and logical operators, and isTruthy() and toString().
//
// Includes full JSON serialization/deserialization logic as well via jsonSerialize() and from_json().
// 
// Includes a std::formatter<Phasor::Value> implementation for use with std::format (or std::print).
// Supports four format specifiers: default (value as-is), t (type name only),
// T (type and value), ? (debug repr with quoted strings and recursive expansion), and
// q (quoted strings, default otherwise).

#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <format>
#ifdef PHASOR_USES_BOOST
#ifdef _DEBUG
#define BOOST_CONTAINER_ENABLE_DEBUG
#endif
#include <boost/variant2.hpp>
#include <boost/container/flat_map.hpp>
#else
#include <variant>
#endif

#include "phsint.hpp"
#include "PhasorString.hpp"

#define phsnull Phasor::Value()

template<typename K, typename V>
struct PhsOrderedMap {
#ifdef PHASOR_USES_BOOST
    boost::container::flat_map<K, size_t> index;
#else
	std::unordered_map<K, size_t> index;
#endif
	std::vector<std::pair<K, V>> data;

    V& operator[](const K& key) {
        auto it = index.find(key);
        if (it != index.end())
            return data[it->second].second;
        index[key] = data.size();
        return data.emplace_back(key, V{}).second;
    }

    auto find(const K& key) {
        auto it = index.find(key);
        if (it != index.end()) return data.begin() + it->second;
        return data.end();
    }

    auto find(const K& key) const {
        auto it = index.find(key);
        if (it != index.end()) return data.begin() + it->second;
        return data.end();
    }

    void erase(const K& key) {
        auto it = index.find(key);
        if (it == index.end()) return;

        size_t pos = it->second;
        index.erase(it);

        if (pos != data.size() - 1) {
            std::swap(data[pos], data.back());
            index[data[pos].first] = pos;
        }
        data.pop_back();
    }

    bool contains(const K& key) const { return index.count(key) > 0; }

    auto end()         { return data.end(); }
    auto end()   const { return data.end(); }
    auto begin()       { return data.begin(); }
    auto begin() const { return data.begin(); }
    [[nodiscard]] bool empty() const { return data.empty(); }
    [[nodiscard]] size_t size() const { return data.size(); }
};

/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/**
 * @brief Runtime value types for the VM
 */
enum class ValueType : u8
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
		Phasor::string structName;
		PhsOrderedMap<Phasor::string, Value> fields;
	};
	using ArrayInstance = std::vector<Value>;

  private:
#ifdef PHASOR_USES_BOOST
	using DataType = boost::variant2::variant<std::monostate, bool, i64, f64, Phasor::string,
	                              std::shared_ptr<StructInstance>,
	                              std::shared_ptr<ArrayInstance>>;
#else
	using DataType = std::variant<std::monostate, bool, i64, f64, Phasor::string,
	                              std::shared_ptr<StructInstance>,
	                              std::shared_ptr<ArrayInstance>>;
#endif

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
	Value(i64 i) : data(i)
	{
	}
	/// @brief Integer constructor
	Value(int i) : data(static_cast<i64>(i))
	{
	}
	/// @brief Double constructor
	Value(f64 d) : data(d)
	{
	}
	/// @brief String constructor
	Value(const std::string &s) : data(Phasor::string(s))
	{
	}
	/// @brief Small Strring constructor
	Value(const Phasor::string &s) : data(s)
	{
	}
	/// @brief String constructor
	Value(const char *s) : data(Phasor::string(s))
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
	/// @brief Struct constructor
	Value(std::initializer_list<std::pair<Phasor::string, Value>> fields)
	{
		auto s = std::make_shared<StructInstance>();
		for (auto& [k, v] : fields)
			s->fields[k] = std::move(v);
		data = std::move(s);
	}

	static Value from_json(const std::string& json);

	static Value from_json(const Phasor::string& json);

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
#ifdef PHASOR_USES_BOOST
		return boost::variant2::holds_alternative<std::shared_ptr<ArrayInstance>>(data);
#else
		return std::holds_alternative<std::shared_ptr<ArrayInstance>>(data);
#endif
	}

	/// @brief Get the value as a boolean
	[[nodiscard]] bool asBool() const noexcept
	{
#ifdef PHASOR_USES_BOOST
		return boost::variant2::get<bool>(data);
#else
		return std::get<bool>(data);
#endif
	}
	/// @brief Get the value as an integer
	[[nodiscard]] i64 asInt() const noexcept
	{
		if (isInt())
		{
#ifdef PHASOR_USES_BOOST
			return boost::variant2::get<i64>(data);
#else
			return std::get<i64>(data);
#endif
		}
		if (isFloat())
		{
#ifdef PHASOR_USES_BOOST
			return static_cast<i64>(boost::variant2::get<f64>(data));
#else
			return static_cast<i64>(std::get<f64>(data));
#endif
		}
        if (isString()) {
#ifdef PHASOR_USES_BOOST
			if (boost::variant2::get<Phasor::string>(data).length() == 1)
				return static_cast<i64>(boost::variant2::get<Phasor::string>(data).c_str()[0]);
#else
			if (std::get<Phasor::string>(data).length() == 1)
				return static_cast<i64>(std::get<Phasor::string>(data).c_str()[0]);
#endif
		}
		
		return 0;
	}
	/// @brief Get an int value as a ASCII char
	[[nodiscard]] Phasor::string intToAscii() const noexcept 
	{
		if (!isInt()) return "";
#ifdef PHASOR_USES_BOOST
		char output[2] = { static_cast<char>(boost::variant2::get<i64>(data)), '\0'};
#else
		char output[2] = { static_cast<char>(std::get<i64>(data)), '\0'};
#endif
		return output;
	}
	/// @brief Get the value as a f64
	[[nodiscard]] f64 asFloat() const noexcept
	{
		if (isFloat())
		{
#ifdef PHASOR_USES_BOOST
			return boost::variant2::get<f64>(data);
#else
			return std::get<f64>(data);
#endif
		}
		if (isInt())
		{
#ifdef PHASOR_USES_BOOST
			return static_cast<f64>(boost::variant2::get<i64>(data));
#else
			return static_cast<f64>(std::get<i64>(data));
#endif
		}
		return 0.0;
	}
	/// @brief Get the value as a STL string
	[[nodiscard]] std::string stl_string() const noexcept
	{
		if (isString())
		{
#ifdef PHASOR_USES_BOOST
			return boost::variant2::get<Phasor::string>(data).str();
#else
			return std::get<Phasor::string>(data).str();
#endif
		}
		return toString();
	}
	/// @brief Get the value as a Phasor String
	[[nodiscard]] Phasor::string string() const noexcept
	{
		if (isString())
		{
#ifdef PHASOR_USES_BOOST
			return boost::variant2::get<Phasor::string>(data);
#else
			return std::get<Phasor::string>(data);
#endif
		}
		return Phasor::string(toString());
	}
	/// @brief Get the value as an array
	std::shared_ptr<ArrayInstance> asArray()
	{
#ifdef PHASOR_USES_BOOST
		return boost::variant2::get<std::shared_ptr<ArrayInstance>>(data);
#else
		return std::get<std::shared_ptr<ArrayInstance>>(data);
#endif
	}

	/// @brief Get the value as an array (const)
	[[nodiscard]] std::shared_ptr<const ArrayInstance> asArray() const noexcept
	{
#ifdef PHASOR_USES_BOOST
		return boost::variant2::get<std::shared_ptr<ArrayInstance>>(data);
#else
		return std::get<std::shared_ptr<ArrayInstance>>(data);
#endif
	}

	[[nodiscard]] bool contains(const std::string& key) const noexcept
	{
		return hasField(Phasor::string(key));
	}

	[[nodiscard]] bool contains(const Phasor::string& key) const noexcept
	{
		return hasField(key);
	}

    [[nodiscard]] Value getField(const char* name) const
    {
        return getField(Phasor::string(name));
    }

    void setField(const char* name, Value value)
    {
        setField(Phasor::string(name), std::move(value));
    }

    [[nodiscard]] bool hasField(const char* name) const noexcept
    {
        return hasField(Phasor::string(name));
    }

    [[nodiscard]] bool contains(const char* key) const noexcept
    {
        return contains(Phasor::string(key));
    }

	[[nodiscard]] Value get_or(const std::string& key, Value fallback) const noexcept
	{
		if (!isStruct()) return fallback;
		auto it = asStruct()->fields.find(Phasor::string(key));
		return it != asStruct()->fields.end() ? it->second : fallback;
	}

	[[nodiscard]] Value get_or(const Phasor::string& key, Value fallback) const noexcept
	{
		if (!isStruct()) return fallback;
		auto it = asStruct()->fields.find(key);
		return it != asStruct()->fields.end() ? it->second : fallback;
	}

	[[nodiscard]] Value get_or(const char* key, Value fallback) const noexcept
	{
		return get_or(Phasor::string(key), std::move(fallback));
	}

	Value operator[](const size_t index) const
	{
#ifdef PHASOR_USES_BOOST
		if (!boost::variant2::holds_alternative<std::shared_ptr<ArrayInstance>>(data))
#else
		if (!std::holds_alternative<std::shared_ptr<ArrayInstance>>(data))
#endif	
		{
			throw std::runtime_error("Value is not an array");
		}

		auto arr = asArray();
		if (index >= arr->size())
			throw std::out_of_range("Array index out of range");
		return (*arr)[index];
	}

	Value& operator[](const size_t index)
	{
		if (isNull())
			data = std::make_shared<ArrayInstance>();

#ifdef PHASOR_USES_BOOST
		if (!boost::variant2::holds_alternative<std::shared_ptr<ArrayInstance>>(data))
#else
		if (!std::holds_alternative<std::shared_ptr<ArrayInstance>>(data))
#endif
		{
			throw std::runtime_error("Value is not an array");
		}

		auto arr = asArray();
		if (index >= arr->size())
			arr->resize(index + 1);

		return (*arr)[index];
	}

	Value& operator[](const Phasor::string& key)
	{
		if (isNull()) {
			data.emplace<std::shared_ptr<StructInstance>>(std::make_shared<StructInstance>());
		}
#ifdef PHASOR_USES_BOOST
		if (!boost::variant2::holds_alternative<std::shared_ptr<StructInstance>>(data))
#else
		if (!std::holds_alternative<std::shared_ptr<StructInstance>>(data))
#endif
		{
			throw std::runtime_error("Value is not a struct");
		}
#ifdef PHASOR_USES_BOOST		
		return boost::variant2::get<std::shared_ptr<StructInstance>>(data)->fields[key];
#else
		return std::get<std::shared_ptr<StructInstance>>(data)->fields[key];
#endif
	}

	Value operator[](const Phasor::string& key) const
	{
#ifdef PHASOR_USES_BOOST
		if (!boost::variant2::holds_alternative<std::shared_ptr<StructInstance>>(data))
#else
		if (std::holds_alternative<std::shared_ptr<StructInstance>>(data))
#endif
		{
			throw std::runtime_error("Value is not a struct");
		}
#ifdef PHASOR_USES_BOOST
		const auto& fields = boost::variant2::get<std::shared_ptr<StructInstance>>(data)->fields;
#else
		const auto& fields = std::get<std::shared_ptr<StructInstance>>(data)->fields;
#endif
		auto it = fields.find(key);
		if (it == fields.end())
			return {};
		return it->second;
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
			return Value(string() + other.string());
		}
		if (isArray() && other.isArray())
		{
			const auto &leftArr = *asArray();
			const auto &rightArr = *other.asArray();
			if (leftArr.size() != rightArr.size())
			{
				throw std::runtime_error("Array sizes must match");
			}

			auto result = std::make_shared<ArrayInstance>();
			result->reserve(leftArr.size());
			for (size_t i = 0; i < leftArr.size(); ++i)
			{
				const Value &lhsElem = leftArr[i];
				const Value &rhsElem = rightArr[i];
				if (lhsElem.isArray() || rhsElem.isArray())
				{
					if (!lhsElem.isArray() || !rhsElem.isArray())
					{
						throw std::runtime_error("Array element types must match");
					}
					result->push_back(lhsElem + rhsElem);
				} else {
					if (lhsElem.getType() != rhsElem.getType())
					{
						throw std::runtime_error("Array element types must match");
					}
					result->push_back(lhsElem + rhsElem);
				}
			}
			return Value(std::move(result));
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
		if (isArray() && other.isArray())
		{
			const auto &leftArr = *asArray();
			const auto &rightArr = *other.asArray();
			if (leftArr.size() != rightArr.size())
			{
				throw std::runtime_error("Array sizes must match");
			}

			auto result = std::make_shared<ArrayInstance>();
			result->reserve(leftArr.size());
			for (size_t i = 0; i < leftArr.size(); ++i)
			{
				const Value &lhsElem = leftArr[i];
				const Value &rhsElem = rightArr[i];
				if (lhsElem.isArray() || rhsElem.isArray())
				{
					if (!lhsElem.isArray() || !rhsElem.isArray())
					{
						throw std::runtime_error("Array element types must match");
					}
					result->push_back(lhsElem - rhsElem);
				} else {
					if (lhsElem.getType() != rhsElem.getType())
					{
						throw std::runtime_error("Array element types must match");
					}
					result->push_back(lhsElem - rhsElem);
				}
			}
			return Value(std::move(result));
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
		if (isArray() && other.isArray())
		{
			const auto &leftArr = *asArray();
			const auto &rightArr = *other.asArray();
			if (leftArr.size() != rightArr.size())
			{
				throw std::runtime_error("Array sizes must match");
			}

			auto result = std::make_shared<ArrayInstance>();
			result->reserve(leftArr.size());
			for (size_t i = 0; i < leftArr.size(); ++i)
			{
				const Value &lhsElem = leftArr[i];
				const Value &rhsElem = rightArr[i];
				if (lhsElem.isArray() || rhsElem.isArray())
				{
					if (!lhsElem.isArray() || !rhsElem.isArray())
					{
						throw std::runtime_error("Array element types must match");
					}
					result->push_back(lhsElem * rhsElem);
				} else {
					if (lhsElem.getType() != rhsElem.getType())
					{
						throw std::runtime_error("Array element types must match");
					}
					result->push_back(lhsElem * rhsElem);
				}
			}
			return Value(std::move(result));
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
		if (isArray() && other.isArray())
		{
			const auto &leftArr = *asArray();
			const auto &rightArr = *other.asArray();
			if (leftArr.size() != rightArr.size())
			{
				throw std::runtime_error("Array sizes must match");
			}

			auto result = std::make_shared<ArrayInstance>();
			result->reserve(leftArr.size());
			for (size_t i = 0; i < leftArr.size(); ++i)
			{
				const Value &lhsElem = leftArr[i];
				const Value &rhsElem = rightArr[i];
				if (lhsElem.isArray() || rhsElem.isArray())
				{
					if (!lhsElem.isArray() || !rhsElem.isArray())
					{
						throw std::runtime_error("Array element types must match");
					}
					result->push_back(lhsElem / rhsElem);
				} else {
					if (lhsElem.getType() != rhsElem.getType())
					{
						throw std::runtime_error("Array element types must match");
					}
					result->push_back(lhsElem / rhsElem);
				}
			}
			return Value(std::move(result));
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
		if (isArray() && other.isArray())
		{
			const auto &leftArr = *asArray();
			const auto &rightArr = *other.asArray();
			if (leftArr.size() != rightArr.size())
			{
				throw std::runtime_error("Array sizes must match");
			}

			auto result = std::make_shared<ArrayInstance>();
			result->reserve(leftArr.size());
			for (size_t i = 0; i < leftArr.size(); ++i)
			{
				const Value &lhsElem = leftArr[i];
				const Value &rhsElem = rightArr[i];
				if (lhsElem.isArray() || rhsElem.isArray())
				{
					if (!lhsElem.isArray() || !rhsElem.isArray())
					{
						throw std::runtime_error("Array element types must match");
					}
					result->push_back(lhsElem % rhsElem);
				} else {
					if (lhsElem.getType() != rhsElem.getType())
					{
						throw std::runtime_error("Array element types must match");
					}
					result->push_back(lhsElem % rhsElem);
				}
			}
			return Value(std::move(result));
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
			return !string().empty();
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
			return string() == other.string();
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
			return string() < other.string();
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
			return string() > other.string();
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

	[[nodiscard]] Phasor::string toRepr() const noexcept
	{
		if (isString())
		{
			return jsonSerialize();
		}
		return toString();
	}

	[[nodiscard]] Phasor::string toString() const noexcept
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
			[[likely]] return stl_string();
		}
		if (isArray())
		{
			return jsonSerialize();
		}
		if (isStruct())
		{
			return jsonSerialize();
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
#ifdef PHASOR_USES_BOOST
		return boost::variant2::get<Phasor::string>(data).c_str();
#else
		return std::get<Phasor::string>(data).c_str();
#endif
	}

	[[nodiscard]] Phasor::string jsonSerialize(int indent = -1, int depth = 0) const
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
			Phasor::string result = "\"";
			for (char c : string())
			{
				switch (c)
				{
				case '\"': result += "\\\""; break;
				case '\\': result += "\\\\"; break;
				case '\b': result += "\\b"; break;
				case '\f': result += "\\f"; break;
				case '\n': result += "\\n"; break;
				case '\r': result += "\\r"; break;
				case '\t': result += "\\t"; break;
				default:
					if (static_cast<unsigned char>(c) < 0x20)
					{
						char buf[7];
						snprintf(buf, sizeof(buf), "\\u%04X", static_cast<unsigned char>(c));
						result += buf;
					} else {
						result += c;
					}
					break;
				}
			}
			result += "\"";
			return result;
		}

		const bool pretty = indent >= 0;

		auto make_indent = [&](int level) -> Phasor::string
		{
			if (!pretty)
			{
				return {};
			}

			const size_t spaces = static_cast<size_t>(level) * static_cast<size_t>(indent);
			return Phasor::string(spaces, ' ');
		};

		if (isArray())
		{
			const auto &arr = *asArray();
			if (arr.empty())
			{
				return "[]";
			}

			Phasor::string result = pretty ? "[\n" : "[";
			const Phasor::string item_indent = make_indent(depth + 1);
			const Phasor::string end_indent = make_indent(depth);

			for (size_t i = 0; i < arr.size(); ++i)
			{
				if (pretty)
				{
					result += item_indent;
				}

				result += arr[i].jsonSerialize(indent, depth + 1).str();

				if (i + 1 < arr.size())
				{
					result += pretty ? ",\n" : ", ";
				}
				else if (pretty)
				{
					result += "\n";
				}
			}

			result += end_indent;
			result += "]";
			return result;
		}

		if (isStruct())
		{
			const auto &s = *asStruct();
			if (s.fields.empty())
			{
				return "{}";
			}

			Phasor::string result = pretty ? "{\n" : "{";
			const Phasor::string item_indent = make_indent(depth + 1);
			const Phasor::string end_indent = make_indent(depth);

			bool first = true;
			for (const auto &[k, v] : s.fields)
			{
				if (!first)
				{
					result += pretty ? ",\n" : ", ";
				}

				if (pretty)
				{
					result += item_indent;
				}

				result += "\"";
				result += k.str();
				result += pretty ? "\": " : "\":";
				result += v.jsonSerialize(indent, depth + 1).str();

				first = false;
			}

			if (pretty)
			{
				result += "\n";
			}
			result += end_indent;
			result += "}";
			return result;
		}

		return "null";
	}

	/// @brief Print to output stream
	friend std::ostream &operator<<(std::ostream &os, const Value &v)
	{
		os << v.toString();
		return os;
	}

	[[nodiscard]] bool isStruct() const
	{
#ifdef PHASOR_USES_BOOST
		return boost::variant2::holds_alternative<std::shared_ptr<StructInstance>>(data);
#else
		return std::holds_alternative<std::shared_ptr<StructInstance>>(data);
#endif
	}

	std::shared_ptr<StructInstance> asStruct()
	{
#ifdef PHASOR_USES_BOOST
		return boost::variant2::get<std::shared_ptr<StructInstance>>(data);
#else
		return std::get<std::shared_ptr<StructInstance>>(data);
#endif
	}

	[[nodiscard]] std::shared_ptr<const StructInstance> asStruct() const noexcept
	{
#ifdef PHASOR_USES_BOOST
		return boost::variant2::get<std::shared_ptr<StructInstance>>(data);
#else
		return std::get<std::shared_ptr<StructInstance>>(data);
#endif
	}

	static Value createStruct(const Phasor::string &name)
	{
		return Value(std::make_shared<StructInstance>(StructInstance{.structName = name, .fields = {}}));
	}

	static Value createArray(std::vector<Value> elements = {})
	{
		return {std::make_shared<ArrayInstance>(std::move(elements))};
	}

	[[nodiscard]] Value getField(const Phasor::string &name) const
	{
#ifdef PHASOR_USES_BOOST
		if (!boost::variant2::holds_alternative<std::shared_ptr<StructInstance>>(data))
#else
		if (!std::holds_alternative<std::shared_ptr<StructInstance>>(data))
#endif
		{
			[[unlikely]] throw std::runtime_error("getField() called on non-struct value");
		}
#ifdef PHASOR_USES_BOOST
		auto s = boost::variant2::get<std::shared_ptr<StructInstance>>(data);
#else
		auto s = std::get<std::shared_ptr<StructInstance>>(data);
#endif
		auto it = s->fields.find(name);
		if (it == s->fields.end())
		{
			return {};
		}
		return it->second;
	}

	void setField(const Phasor::string &name, Value value)
	{
#ifdef PHASOR_USES_BOOST
		if (!boost::variant2::holds_alternative<std::shared_ptr<StructInstance>>(data))
#else
		if (!std::holds_alternative<std::shared_ptr<StructInstance>>(data))
#endif
		{
			[[unlikely]] throw std::runtime_error("setField() called on non-struct value");
		}
#ifdef PHASOR_USES_BOOST
		auto s = boost::variant2::get<std::shared_ptr<StructInstance>>(data);
#else
		auto s = std::get<std::shared_ptr<StructInstance>>(data);
#endif
		s->fields[name] = std::move(value);
	}

	[[nodiscard]] bool hasField(const Phasor::string &name) const noexcept
	{
#ifdef PHASOR_USES_BOOST
		if (!boost::variant2::holds_alternative<std::shared_ptr<StructInstance>>(data))
#else
		if (!std::holds_alternative<std::shared_ptr<StructInstance>>(data))
#endif
		{
			return false;
		}
#ifdef PHASOR_USES_BOOST
		auto s = boost::variant2::get<std::shared_ptr<StructInstance>>(data);
#else
		auto s = std::get<std::shared_ptr<StructInstance>>(data);
#endif
		return s->fields.contains(name);
	}
};

} // namespace Phasor

#include "PhasorJson.hpp"

namespace Phasor {

inline Value Value::from_json(const Phasor::string& json) {
    std::string_view sv(json);
    auto it = sv.begin();
    auto end = sv.end();
    Value result = PhsJson::parse_value(it, end);
	PhsJson::skip_whitespace(it, end);
    if (it != end)
        throw std::runtime_error("Extra characters after JSON value");
    return result;
}

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
			return fwd(Value::typeToString(v.getType()).string().str());

		case Style::TypeValue:
			return fwd(Value::typeToString(v.getType()).string().str() + "(" + escapeString(v.toString()) + ")");

		case Style::Debug:
			return fwd(debug_repr(v));

		case Style::Quoted:
			if (v.isString())
			{
				return fwd("\"" + escapeString(v.string()) + "\"");
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
				return fwd(v.string());
			case ValueType::Array:
				return fwd(v.toString());
			case ValueType::Struct:
				return fwd(v.toString());
			}
		}
		return ctx.out();
	}

  private:
	static Phasor::string escapeString(std::string_view input)
	{
		Phasor::string output;
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
				} else {
					output += c;
				}
				break;
			}
		}
		return output;
	}

	static Phasor::string debug_repr(const Phasor::Value &v)
	{
		using Phasor::ValueType;
		switch (v.getType())
		{
		case ValueType::Null:
			return "null";
		case ValueType::String:
			return "\"" + escapeString(v.string()) + "\"";
		case ValueType::Array: {
			const auto &arr = *v.asArray();
			Phasor::string out = "[";
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
			Phasor::string out = s.structName.str() + " { ";
			bool        first = true;
			for (const auto &[k, val] : s.fields)
			{
				if (!first)
				{
					out += ", ";
				}
				out += k.str() + ": " + debug_repr(val);
				first = false;
			}
			return out + " }";
		}
		default:
			return v.toString();
		}
	}
};