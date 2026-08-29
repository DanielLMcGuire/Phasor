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
#include <memory_resource>
#include <vformat.hpp>
#include "../phsint.hpp"

#ifndef SANDBOXED
#include "PhasorFFI.hpp"
#endif
#include "PhasorISA.hpp"
#include "../Value.hpp"
#include <platform.h>

#define BAD_STATUS -1

/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

struct ManagedTraceEntry
{
	Phasor::string functionName;
    Value pc;
	Value::ArrayInstance args;
    std::array<Value, 3> registers;

	[[nodiscard]] inline Phasor::string formatTrace() const
	{
		Phasor::string entry = vformat::format(
			"PC=%s FUNC=%s ARGS=%s R0=%s R1=%s R2=%s",
			pc.toRepr().c_str(),
			Value(functionName).toRepr().c_str(),
			Value::createArray(args).toRepr().c_str(),
			registers[0].toRepr().c_str(),
			registers[1].toRepr().c_str(),
			registers[2].toRepr().c_str()
		);

		return entry;
	}
};

class ManagedTraceLog
{
private:
    std::vector<ManagedTraceEntry> stack;

public:
    void push(const ManagedTraceEntry& entry)
    {
        stack.push_back(entry);
    }

    bool pop(ManagedTraceEntry& out)
    {
        if (stack.empty())
            return false;

        out = stack.back();
        stack.pop_back();
        return true;
    }

	bool pop()
    {
        if (stack.empty())
            return false;

        stack.pop_back();
        return true;
    }

    size_t size() const
    {
        return stack.size();
    }

    void clear()
    {
        stack.clear();
    }

	[[nodiscard]] inline Phasor::string format() const
	{
		Phasor::string result;
		size_t counter = 0;

		for (const auto& entry : stack)
		{
			result += ' ';
			result += std::to_string(counter);
			result += "# ";
			result += entry.formatTrace();
			result += '\n';
			counter++;
		}

		return result;
	}
};

/// @class VM
/// @brief Virtual Machine
class VM
{
  public:
	VM();
	VM(const Bytecode &bytecode);
	VM(const OpCode &op, const int &operand1 = 0, const int &operand2 = 0, const int &operand3 = 0);
	~VM();

	/// @brief Initialize the FFI plugins
	void initFFI(const std::vector<std::filesystem::path> &paths);

	/// @brief Get Phasor VM version
	Phasor::string getVersion();

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
	int run(const Bytecode &bytecode, const size_t startPC = 0);

	/// @brief Run a function from bytecode on the virtual machine
	Value runFunction(const Phasor::string &name, const Bytecode &bytecode, const bool &argsInit = false);

	/// @brief Native function signature
	using NativeFunction = std::function<Value(const Value::ArrayInstance &args, VM *vm)>;

	/// @brief Register a native function
	void registerNativeFunction(const Phasor::string &name, NativeFunction fn);

	/// @brief Free a variable in the VM
	void freeVariable(size_t index);

	/// @brief Free a variable by name in the VM
	void freeVariableByName(const Phasor::string &name);

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
	void setRegister(u8 index, const Value &value);

	/// @brief Free a register (reset to null)
	/// @param index Register index to free
	void freeRegister(u8 index);

	/// @brief Get a register value
	/// @param index Register index
	/// @return Value in the register
	Value getRegister(u8 index);

	/// @brief Get the total number of registers
	/// @return Number of registers
	size_t getRegisterCount();

	inline Bytecode getBytecode() {
		return *m_bytecode;
	}

	/// @brief Enum for registers
	enum Register : u8
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
	Value __fastcall operation(const OpCode &op, const int &operand1 = 0, const int &operand2 = 0,
	                           const int &operand3 = 0);
#else
	/// @brief Execute a single operation
	Value operation(const OpCode &op, const int &operand1 = 0, const int &operand2 = 0, const int &operand3 = 0);
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
	Phasor::string getInformation();

	/// @brief Get bytecode information for debugging
	Phasor::string getBytecodeInformation();

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
	int  getStatus();
	bool isErrorStatus();

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
		operation(opcode);
		return getRegister(REGISTER1);
	}

	/**
	 * @brief Run an opcode with values pushed to the stack
	 * @tparam Args Argument types
	 * @param opcode Opcode to run
	 * @param args Arguments to push to the stack
	 * @return Value returned to stack
	 */
	template <typename... Args> inline Value stackRun(OpCode opcode, Args &&...args)
	{
		Value arr[] = {Value(std::forward<Args>(args))...};
		for (Value &v : arr | std::views::reverse)
			push(v);
		operation(opcode);
		return pop();
	}

  private:
	void setup(const Bytecode &bc, const size_t initialPC);
	void evalLoop();

	bool isDirectCall = false; ///< is a direct call to a function

#ifndef SANDBOXED
	/// @brief FFI
	std::unique_ptr<FFI> ffi;
#endif
	/// @brief Exit code
	int status = 0;

	/// @brief Is status an error code
	bool isError = false;

	/// @brief Virtual registers for register-based operations (v2.0)
	std::array<Value, MAX_REGISTERS> registers;
	
	/// @brief Stack
	std::pmr::monotonic_buffer_resource stack_pool;
	std::pmr::vector<Value> stack;

	/// @brief Call stack for function calls
	std::vector<int> callStack;

	/// @brief Variable storage indexed by variable index, or simply: the managed heap
	Value::ArrayInstance variables;

	/// @brief Bytecode to execute
	const Bytecode *m_bytecode{};

	/// @brief Program counter
	size_t pc = 0;

	/// @brief Native function registry
	std::map<Phasor::string, NativeFunction> nativeFunctions;

	ManagedTraceLog tracelog;
};
} // namespace Phasor