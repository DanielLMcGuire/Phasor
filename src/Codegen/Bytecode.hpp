#pragma once
#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <phsint.hpp>
#include "../ISA/ISA.hpp"
#include <Value.hpp>

namespace Phasor
{
/// @brief Instruction with up to 5 operands
/// Format: instruction operand1, operand2, operand3
/// Each instruction uses only the operands it needs
struct Instruction
{
	OpCode  op;       ///< Operation code
	i32 operand1; ///< First operand
	i32 operand2; ///< Second operand
	i32 operand3; ///< Third operand

	// Default constructor
	Instruction() : op(OpCode::HALT), operand1(0), operand2(0), operand3(0)
	{
	}

	// Full constructor
	Instruction(OpCode op, i32 op1 = 0, i32 op2 = 0, i32 op3 = 0)
	    : op(op), operand1(op1), operand2(op2), operand3(op3)
	{
	}
};

/// @brief Struct metadata stored alongside bytecode (struct section)
struct StructInfo
{
    std::string              name;
    int                      firstConstIndex;
    int                      fieldCount;
    std::vector<std::string> fieldNames;
    std::vector<std::vector<int>> fieldArrayDims;
    std::vector<std::string>      fieldTypeNames;
};

/// @brief Complete bytecode structure
struct Bytecode
{
	std::vector<Instruction>             instructions;        ///< List of instructions
	std::vector<Value>                   constants;           ///< Constant pool
	std::unordered_map<std::string, int> variables;           ///< Variable name -> index mapping
	std::vector<std::vector<std::pair<int, std::string>>> scopeVarLists; ///< Per-scope var indices to free on EXIT_SCOPE
	std::unordered_map<std::string, int> functionEntries;     ///< Function name -> instruction index mapping
	std::unordered_map<std::string, int> functionParamCounts; ///< Function name -> parameter count
	std::unordered_map<std::string, std::vector<std::string>> functionParamTypeNames; ///< Function name -> parameter type names
	std::unordered_map<std::string, std::vector<std::vector<int>>> functionParamArrayDims;
	std::unordered_map<std::string, std::string> functionReturnTypeNames; ///< Function name -> return type name
	std::unordered_map<std::string, std::vector<int>> functionReturnArrayDims; ///< Function name -> return array dims
	int                                  nextVarIndex = 0;    ///< Next available variable index

	// Struct section (planned usage by future struct codegen)
	std::vector<StructInfo>              structs;       ///< List of struct descriptors
	std::unordered_map<std::string, int> structEntries; ///< Struct name -> index in structs

	/// @brief Add a constant to the pool and return its index
	int addConstant(const Value &value)
	{
		constants.push_back(value);
		return static_cast<int>(constants.size()) - 1;
	}

	/// @brief Add a string constant with deduplication
	int addStringConstant(const std::string &s)
	{
		auto it = stringConstantCache.find(s);
		if (it != stringConstantCache.end())
		{
			return it->second;
		}
		int idx = addConstant(Value(s));
		stringConstantCache[s] = idx;
		return idx;
	}

	std::unordered_map<std::string, int> stringConstantCache; ///< Dedup cache for string constants

	/// @brief Get or create a variable index
	int getOrCreateVar(const std::string &name)
	{
		auto it = variables.find(name);
		if (it != variables.end())
		{
			return it->second;
		}
		int index = nextVarIndex++;
		variables[name] = index;
		return index;
	}

	/// @brief Emit an instruction with operands
	void emit(OpCode op, i32 op1 = 0, i32 op2 = 0, i32 op3 = 0)
	{
		instructions.emplace_back(op, op1, op2, op3);
	}
};
}