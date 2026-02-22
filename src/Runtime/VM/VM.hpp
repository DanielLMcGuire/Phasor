#pragma once
#include "../../Codegen/CodeGen.hpp"
#include "../Value.hpp"
#include "Runtime.hpp"
#include <vector>
#include <filesystem>
#include <functional>
#include <map>
#include <array>
#include "core/core.h"
#include <iostream>
#include <stdexcept>

/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

/// @class VM
/// @brief Virtual Machine — executes bytecode within a given Instance and Frame.
///        VM no longer owns execution state directly; pc, stack, callStack, and
///        registers all live in Frame. The VM holds a pointer to the active
///        Instance (owned by Runtime) and operates through it.
class VM
{
  public:
	explicit VM() = default;
	~VM()
	{
		flush();
		flusherr();
	}

	/// @brief Execute bytecode within the given instance.
	///        Called by Runtime — do not call directly unless you know what you are doing.
	/// @return Exit status code
	int run(Instance &instance);

	/// @brief Native function signature
	using NativeFunction = std::function<Value(const std::vector<Value> &args, VM *vm)>;

	/// @brief Register a native function
	void registerNativeFunction(const std::string &name, NativeFunction fn);

	/// @brief Free a variable in the current instance
	void freeVariable(const size_t index);

	/// @brief Add a variable to the current instance
	/// @return Index of the new variable
	size_t addVariable(const Value &value);

	/// @brief Set a variable in the current instance
	void setVariable(const size_t index, const Value &value);

	/// @brief Get a variable from the current instance
	Value getVariable(const size_t index);

	/// @brief Get the number of variables in the current instance
	size_t getVariableCount();

	/// @brief Set a register value in the active frame
	void setRegister(uint8_t index, const Value &value);

	/// @brief Free a register (reset to null) in the active frame
	void freeRegister(uint8_t index);

	/// @brief Get a register value from the active frame
	Value getRegister(uint8_t index);

	/// @brief Get the total number of registers per frame
	size_t getRegisterCount();

	/// @brief Named register aliases (r0–r31)
	enum Register
	{
		r0,  r1,  r2,  r3,
		r4,  r5,  r6,  r7,
		r8,  r9,  r10, r11,
		r12, r13, r14, r15,
		r16, r17, r18, r19,
		r20, r21, r22, r23,
		r24, r25, r26, r27,
		r28, r29, r30, r31
		// 32 registers per frame. If you need more, I highly suggest that you reconsider.
	};

	/// @brief Thrown internally to halt execution cleanly
	class Halt : public std::exception
	{
	  public:
		const char *what() const noexcept override { return ""; }
	};

#ifdef _WIN32
	/// @brief Execute a single operation
	Value __fastcall operation(const OpCode &op, const int &operand1 = 0, const int &operand2 = 0,
	                           const int &operand3 = 0, const int &operand4 = 0, const int &operand5 = 0);
#else
	/// @brief Execute a single operation
	Value operation(const OpCode &op, const int &operand1 = 0, const int &operand2 = 0,
	                const int &operand3 = 0, const int &operand4 = 0, const int &operand5 = 0);
#endif

	/// @brief Push a value onto the active frame's stack
	void push(const Value &value);

	/// @brief Pop a value from the active frame's stack
	Value pop();

	/// @brief Peek at the top value on the active frame's stack
	Value peek();

	/// @brief Get diagnostic information for the current execution state
	std::string getInformation(Instance &instance);

	/// @brief Log a value to stdout via the runtime print path
	void log(const Value &msg);

	/// @brief Log a value to stderr
	void logerr(const Value &msg);

	void flush();
	void flusherr();

	/// @brief Exit status of the last run() call
	int status = 0;

  private:
	/// @brief Active instance — set at the start of run(), cleared on return
	Instance *m_instance = nullptr;

	/// @brief Native function registry — shared across runs on this VM
	std::map<std::string, NativeFunction> nativeFunctions;

	/// @brief Convenience accessor: active frame (top of instance call stack)
	Frame &activeFrame();
};

} // namespace Phasor