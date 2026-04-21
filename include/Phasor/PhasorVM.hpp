// Copyright 2026 Daniel McGuire
// Licensed under the Apache License (with LLVM-Exceptions), Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// https://llvm.org/LICENSE.txt
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// README
// 
// Because the VM is such a complex component, this readme only covers
// basic high-level use cases.
// For more information, please refer to the below, and the internal header 
// at src/Runtime/VM/VM.hpp, as well as the [doxygen](phasor-docs.pages.dev)
//
// Usage:
// ```cpp
// // Initialize VM
// Phasor::VM vm;
// // Run bytecode
// vm.run(bytecode);
// ```

#pragma once

#include <vector>
#include <filesystem>
#include <functional>
#include <map>
#include <array>
#include <ranges>
#include <iostream>
#include <stdexcept>

#ifndef SANDBOXED
	#include "PhasorFFI.hpp"
#endif
#include "PhasorISA.hpp"
#include "../Value.hpp"

/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/// @class VM
/// @brief Virtual Machine
class VM
{
  public:
	VM();
	VM(const Bytecode &bytecode);
	VM(const OpCode &op, const int &operand1 = 0, const int &operand2 = 0, const int &operand3 = 0);
	~VM();

	inline void initFFI(const std::filesystem::path &path);

	/// @brief Get Phasor VM version
	std::string getVersion();

	/// @class Halt
	/// @brief Throws when the HALT opcode is reached
	class Halt : public std::exception
	{
	  public:
		const char *what() const noexcept override
		{
			return "";
		}
	};

	/// @brief Run the virtual machine
	/// Exits -1 on uncaught exception
	int run(const Bytecode &bytecode);

	/// @brief Native function signature
	using NativeFunction = std::function<Value(const std::vector<Value> &args, VM *vm)>;

	/// @brief Register a native function
	void registerNativeFunction(const std::string &name, NativeFunction fn);

	using ImportHandler = std::function<void(const std::filesystem::path &path)>;
	/// @brief Set the import handler for importing modules
	void setImportHandler(const ImportHandler &handler);

	/// @brief Free a variable in the VM
	void freeVariable(size_t index);

	/// @brief Add a variable to the VM
	/// @param value The value to add
	/// @return The index of the variable
	size_t addVariable(const Value &value);

	/// @brief Set a variable in the VM
	/// @param index The index of the variable
	/// @param value The value to set
	void setVariable(size_t index, const Value &value);

	/// @brief Get a variable from the VM
	Value getVariable(size_t index);

	/// @brief Get the number of variables in the VM
	size_t getVariableCount();

	/// @brief Set a register value
	/// @param index Register index
	/// @param value Value to set
	void setRegister(uint8_t index, const Value &value);

	/// @brief Free a register (reset to null)
	/// @param index Register index to free
	void freeRegister(uint8_t index);

	/// @brief Get a register value
	/// @param index Register index
	/// @return Value in the register
	Value getRegister(uint8_t index);

	/// @brief Get the total number of registers
	/// @return Number of registers
	size_t getRegisterCount();

	/// @brief Enum for registers
	enum Register : uint8_t
	{
		r0,
		r1,
		r2,
		r3,
		r4,
		r5,
		r6,
		r7,
		r8,
		r9,
		r10,
		r11,
		r12,
		r13,
		r14,
		r15,
		r16,
		r17,
		r18,
		r19,
		r20,
		r21,
		r22,
		r23,
		r24,
		r25,
		r26,
		r27,
		r28,
		r29,
		r30,
		r31
	};

	#define REGISTER1 VM::Register::r0
	#define REGISTER2 VM::Register::r1
	#define REGISTER3 VM::Register::r2

#ifdef _WIN32
	/// @brief Execute a single operation
	inline Value __fastcall operation(const OpCode &op, const int &operand1 = 0, const int &operand2 = 0, const int &operand3 = 0);
#else
	/// @brief Execute a single operation
	inline Value operation(const OpCode &op, const int &operand1 = 0, const int &operand2 = 0, const int &operand3 = 0);
#endif
	/// @brief Push a value onto the stack
	void push(const Value &value);

	/// @brief Pop a value from the stack
	Value pop();

	/// @brief Peek at the top value on the stack
	Value peek();

	/// @brief Clean up the virtual machine
	void cleanup();

	/// @brief Reset the virtual machine
	void reset(const bool &resetStack = true, const bool &resetFunctions = true, const bool &resetVariables = true);

	/// @brief Get VM information for debugging
	std::string getInformation();

	/// @brief Get bytecode information for debugging
	std::string getBytecodeInformation();

	/// @brief Log a Value to stdout
	void log(const Value &msg);

	/// @brief Log a Value to stderr
	void logerr(const Value &msg);

	/// @brief Flush stdout
	void flush();

	/// @brief Flush stderr
	void flusherr();
	
	/// @brief Set VM exit code
	void setStatus(int newStatus);
	void resetStatus();
	int getStatus();

	/** 
	 * @brief Run an opcode with arguments pre-loaded into registers
	 * @tparam Args Argument types
	 * @param opcode Opcode to run
	 * @param args Arguments to load into registers
	 * @return Return value of the operation
	*/
	template <typename... Args> inline Value regRun(OpCode opcode, Args &&...args)
	{
		int regIndex = 0;
		(setRegister(regIndex++, std::forward<Args>(args)), ...);
		return operation(opcode);
	}

	/**
	 * @brief Run an opcode with values pushed to the stack
	 * @tparam Args Argument types
	 * @param opcode Opcode to run
	 * @param args Arguments to push to the stack
	 * @return Value returned to stack
	 */
	template <typename... Args> inline Value stackRun(OpCode opcode, Args&&... args) {
		Value arr[] = {Value(std::forward<Args>(args))...};
		for (Value& v : arr | std::views::reverse)
			push(v);
		operation(opcode);
		return pop();
	}
};
} // namespace Phasor