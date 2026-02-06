#pragma once
#include "../../Codegen/CodeGen.hpp"
#include "../Value.hpp"
#include <vector>
#include <filesystem>
#include <functional>
#include <map>
#include <array>
#include "core/core.h"

/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/// @class VM
/// @brief Virtual Machine
class VM
{
  public:
	/// @brief Run the virtual machine
	void run(const Bytecode &bytecode);

	/// @brief Native function signature
	using NativeFunction = std::function<Value(const std::vector<Value> &args, VM *vm)>;

	/// @brief Register a native function
	void registerNativeFunction(const std::string &name, NativeFunction fn);

	using ImportHandler = std::function<void(const std::filesystem::path &path)>;
	void setImportHandler(ImportHandler handler);

	/// @brief Free a variable in the VM
	void freeVariable(const size_t index);

	/// @brief Add a variable to the VM
	/// @param value The value to add
	/// @return The index of the variable
	size_t addVariable(const Value &value);

	/// @brief Set a variable in the VM
	/// @param index The index of the variable
	/// @param value The value to set
	void setVariable(const size_t index, const Value &value);

	/// @brief Get a variable from the VM
	Value getVariable(const size_t index);

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

	/// @brief Enum for lower 26 registers
	enum Register
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
		r25 // If you are here to try adding more, step back and think about what got you here in the first place
	};
#ifdef _WIN32
	/// @brief Execute a single operation
	Value __fastcall operation(const OpCode &op, const int &operand1 = 0, const int &operand2 = 0,
	                          const int &operand3 = 0,
	                const int &operand4 = 0, const int &operand5 = 0);
#else
		/// @brief Execute a single operation
	Value operation(const OpCode &op, const int &operand1 = 0, const int &operand2 = 0,
	                          const int &operand3 = 0,
	                const int &operand4 = 0, const int &operand5 = 0);
#endif
	/// @brief Push a value onto the stack
	void push(const Value &value);

	/// @brief Pop a value from the stack
	Value pop();

	/// @brief Peek at the top value on the stack
	Value peek();

	/// @brief Reset the virtual machine
	void reset(const bool &resetStack = true, const bool &resetFunctions = true, const bool &resetVariables = true);

	/// @brief Get VM information for debugging
	std::string getInformation();

	/// @brief Use the VM's logging via print opcode
	void log(const Value &msg);
	void logerr(const Value &msg);

  private:
	/// @brief Import handler for loading modules
	ImportHandler importHandler;

	/// @brief Virtual registers for register-based operations (v2.0)
	std::array<Value, 256> registers;

	/// @brief Stack for function calls
	std::vector<Value> stack;

	/// @brief Call stack for function calls
	std::vector<int> callStack;

	/// @brief Variable storage indexed by variable index
	std::vector<Value> variables;

	/// @brief Bytecode to execute
	const Bytecode *bytecode;

	/// @brief Program counter
	size_t pc = 0;

	/// @brief Native function registry
	std::map<std::string, NativeFunction> nativeFunctions;
};
} // namespace Phasor
