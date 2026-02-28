#pragma once
#include "../../Codegen/CodeGen.hpp"
#include <Value.hpp>
#include <vector>
#include <filesystem>
#include <functional>
#include <map>
#include <array>
#include <memory>
#include <limits>
#include "core/core.h"
#include <iostream>
#include <stdexcept>

/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

using InstanceHandle = std::size_t;
static constexpr InstanceHandle NULL_HANDLE = std::numeric_limits<InstanceHandle>::max();

enum class ErrorStatus
{
	Null,
	RuntimeError,
	StackUnderflow,
	InvalidOpcode,
	ModuleNotFound,
	ChecksumMismatch,
	EntryNotFound,
	AccessViolation,
};

struct Frame
{
	std::vector<Value>    stack;
	std::size_t           pc = 0;
	std::array<Value, 32> registers;
	InstanceHandle        returnToInstance = NULL_HANDLE;
	std::size_t           returnPc = 0;

	Frame()
	{
		registers.fill(Value());
	}
};

struct Instance
{
	Bytecode                    code;
	std::vector<Frame>          callStack;
	std::vector<Value>          variables;
	bool                        alive = true;
	std::vector<InstanceHandle> imports;
	ErrorStatus                 error = ErrorStatus::Null;
	std::string                 errorMsg;

	Frame &activeFrame()
	{
		return callStack.back();
	}
	const Frame &activeFrame() const
	{
		return callStack.back();
	}

	void pushFrame(InstanceHandle returnTo = NULL_HANDLE, std::size_t returnPc = 0)
	{
		Frame f;
		f.returnToInstance = returnTo;
		f.returnPc = returnPc;
		callStack.push_back(std::move(f));
	}

	Frame popFrame()
	{
		Frame f = std::move(callStack.back());
		callStack.pop_back();
		return f;
	}
};

/// @class VM
/// @brief Virtual Machine and Runtime — owns all instances, the module cache,
///        and the execution engine in a single entity.
class VM
{
  public:
	explicit VM() = default;
	~VM()
	{
		flush();
		flusherr();
	}

	/// @brief Native function signature
	using NativeFunction = std::function<Value(const std::vector<Value> &args, VM *vm)>;

	/// @brief Register a native function
	void registerNativeFunction(const std::string &name, NativeFunction fn);

	/// @brief Create an empty instance and return its handle
	InstanceHandle createInstance();

	/// @brief Load bytecode into a new instance and return its handle
	InstanceHandle load(const Bytecode &code, InstanceHandle owner = NULL_HANDLE);

	/// @brief Run an instance to completion
	/// @return Exit status
	int execute(InstanceHandle handle);

	/// @brief Resolve a handle to a live Instance pointer. Returns nullptr if invalid.
	Instance *resolve(InstanceHandle handle);

	/// @brief Destroy an instance and release its memory
	void destroyInstance(InstanceHandle handle);

	
	void   freeVariable(const size_t index);
	size_t addVariable(const Value &value);
	void   setVariable(const size_t index, const Value &value);
	Value  getVariable(const size_t index);
	size_t getVariableCount();

	void   setRegister(uint8_t index, const Value &value);
	void   freeRegister(uint8_t index);
	Value  getRegister(uint8_t index);
	size_t getRegisterCount();

	/// @brief Enum for registers
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

	/// @brief Thrown internally to halt execution cleanly
	class Halt : public std::exception
	{
	  public:
		const char *what() const noexcept override
		{
			return "";
		}
	};

#ifdef _WIN32
	Value __fastcall operation(const OpCode &op, const int &operand1 = 0, const int &operand2 = 0,
	                           const int &operand3 = 0, const int &operand4 = 0, const int &operand5 = 0);
#else
	Value operation(const OpCode &op, const int &operand1 = 0, const int &operand2 = 0, const int &operand3 = 0,
	                const int &operand4 = 0, const int &operand5 = 0);
#endif

	void  push(const Value &value);
	Value pop();
	Value peek();

	std::string getInformation();
	std::string getRuntimeInformation() const;

	void log(const Value &msg);
	void logerr(const Value &msg);
	void flush();
	void flusherr();

	int status = 0;

	template <typename... Args> inline Value regRun( OpCode opcode, Args &&...args)
	{
		int regIndex = 0;
		(setRegister(regIndex++, std::forward<Args>(args)), ...);
		return operation(opcode);
	}

  private:
	/// @brief All live instances — index == handle
	std::vector<std::unique_ptr<Instance>> m_instances;

	/// @brief Handle of the currently executing instance
	InstanceHandle m_current = NULL_HANDLE;

	/// @brief Active instance during run() — set at entry, cleared on exit
	Instance *m_instance = nullptr;

	/// @brief Native function registry
	std::map<std::string, NativeFunction> nativeFunctions;

	/// @brief Convenience accessor: active frame
	Frame &activeFrame()
	{
		return m_instance->activeFrame();
	}

	/// @brief Internal execution loop — drives a single instance
	int run(Instance &instance);
};

} // namespace Phasor