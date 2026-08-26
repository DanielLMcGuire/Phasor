#ifndef CMAKE_PCH
#include "VM.hpp"
#endif
#include <iostream>
#include <stdexcept>
#include <format>
#include <cassert>
#include "core/core.h"
#include <phsint.hpp>
#include "../../ISA/map.hpp"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

#ifdef TIMING
#include <chrono>
#endif

#ifdef _DEBUG
#ifdef _WIN32
inline bool isDebuggerAttached()
{
	return IsDebuggerPresent() == TRUE;
}
#else
#include <unistd.h>
#include <sys/ptrace.h>
inline bool isDebuggerAttached()
{
	return ptrace(PTRACE_TRACEME, 0, 1, 0) == -1;
}
#endif
#endif
#include <version>

#ifdef PHASOR_USES_BOOST
    #ifdef _WIN32
    	#define BOOST_STACKTRACE_USE_WINDBG
    #endif
    #include <boost/stacktrace.hpp>
	#include <boost/assert/source_location.hpp>
	#define PHS_SRC_LOC() (std::format("{} @ {}:{}", BOOST_CURRENT_LOCATION.function_name(), BOOST_CURRENT_LOCATION.file_name(), BOOST_CURRENT_LOCATION.line()))
#elif defined(__cpp_lib_stacktrace) && __cpp_lib_stacktrace >= 202011L
	#include <stacktrace>
	#define PHS_SRC_LOC() (std::format("VM::{}()", __func__))
#endif

namespace Phasor
{

PhsString VM::getVersion()
{
	return PHASOR_VERSION_STRING;
}
void VM::setStatus(int newStatus)
{
	status = newStatus;
}
void VM::resetStatus()
{
	status = 0;
}
int VM::getStatus()
{
	return status;
}
bool VM::isErrorStatus()
{
	return isError;
}

void VM::initFFI(const std::vector<std::filesystem::path> &paths)
{
#ifndef SANDBOXED
	ffi = std::make_unique<FFI>(paths, this);
#endif
}

void VM::setup(const Bytecode &bc, const size_t initialPC)
{
	m_bytecode = &bc;
	pc = initialPC;
	stack.clear();
	callStack.clear();
	tracelog.clear();

#ifdef TRACING
	log(std::format("\n{}:\n\n{}\n", PHS_SRC_LOC(), getBytecodeInformation()));
	flush();
#endif

	registers.fill(Value());
	variables.resize(m_bytecode->nextVarIndex);
}

int VM::run(const Bytecode &bc, const size_t startPC)
{
	setup(bc, startPC);
#ifdef TRACING
	bool singleInstruction = false;
#endif
	tracelog.push({"<init>", 0, {}, {phsnull, phsnull, phsnull}});

	if (m_bytecode->instructions.size() < 2) { [[unlikely]]
		if (m_bytecode->instructions.size() == 1) {
			Instruction instruction = m_bytecode->instructions[0];
			std::vector<std::string> operandParts;
			if (instruction.operand3 != 0) {
				operandParts.push_back(std::to_string(instruction.operand1));
				operandParts.push_back(std::to_string(instruction.operand2));
				operandParts.push_back(std::to_string(instruction.operand3));
			} else if (instruction.operand2 != 0) {
				operandParts.push_back(std::to_string(instruction.operand1));
				operandParts.push_back(std::to_string(instruction.operand2));
			} else if (instruction.operand1 != 0) {
				operandParts.push_back(std::to_string(instruction.operand1));
			}
			std::string operands;
			for (size_t i = 0; i < operandParts.size(); ++i) {
				if (i != 0) operands += ", ";
				operands += operandParts[i];
			}
			log(std::format("Warning! Bytecode has 1 instruction:\n{} {}", opCodeToString(instruction.op), operands));
			flush();
#ifdef TRACING
			singleInstruction = true;
#endif
		} else {
			log(std::format("Warning! Bytecode has 0 instructions."));
			flush();
			return 0;
		}
	}

#ifdef TRACING
	log(std::format("\n{}:\n\n", PHS_SRC_LOC()));
	flush();
#endif

#ifdef TIMING
	using clock = std::chrono::high_resolution_clock;
	auto start = clock::now();
#endif

#ifdef _WIN32
	int out_fd = _fileno(stdout);
    int in_fd = _fileno(stdin);
	int old_out_mode = _setmode(out_fd, _O_BINARY);
    int old_in_mode = _setmode(in_fd, _O_BINARY);
#endif

	try
	{
		evalLoop();
		return status;
	}
	catch (const VM::Halt &)
	{
#ifdef _WIN32
	_setmode(out_fd, old_out_mode);
    _setmode(in_fd, old_in_mode);
#endif
#ifdef TIMING
		auto end = clock::now();
		auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		log(std::format("{}: Duration of bytecode execution: {}us\n\n", PHS_SRC_LOC(), us));
		flush();
#endif
#ifdef TRACING
		std::ostringstream stacklog;
#ifdef PHASOR_USES_BOOST
		stacklog << boost::stacktrace::stacktrace();
#elif defined(__cpp_lib_stacktrace) && __cpp_lib_stacktrace >= 202011L
		stacklog << std::stacktrace::current();
#else
		stacklog << "[No C++23 <stacktrace> support]";
#endif
		if (singleInstruction) { [[unlikely]]
			log(std::format("\nUser code exited {} with code {}\n", status == 0 ? "\x1B[0;32msuccessfully\x1B[0m" : "\x1B[0;31mabnormally\x1B[0m", status));
			flush();
		} else {
			log(std::format("\n{}: CAUGHT Phasor:VM::Halt\n\n{}\n\n{}\n\nUser code exited {} with code {}\n", PHS_SRC_LOC(), stacklog.str(), getInformation(), status == 0 ? "\x1B[0;32msuccessfully\x1B[0m" : "\x1B[0;31mabnormally\x1B[0m", status));
			flush();
		}
#endif
#ifdef _DEBUG
		if (isDebuggerAttached())
			assert(status == 0);
#endif
		return status;
	}
	catch (const std::exception &e)
	{
#ifdef _WIN32
	_setmode(out_fd, old_out_mode);
    _setmode(in_fd, old_in_mode);
#endif
		std::ostringstream stacklog;
#ifdef PHASOR_USES_BOOST
		stacklog << boost::stacktrace::stacktrace();
#elif defined(__cpp_lib_stacktrace) && __cpp_lib_stacktrace >= 202011L
		stacklog << std::stacktrace::current();
#else
		stacklog << "[No C++23 <stacktrace> support]";
#endif
		logerr(std::format("\n{}: \x1B[0;31mUNCAUGHT std::exception occured in Phasor VM Runtime\x1B[0m\n\n{}\n\n{}\n\nMANAGED:\n{}\n\nNATIVE:\n{}\n\n", PHS_SRC_LOC(), e.what(), getInformation(), tracelog.format(), stacklog.str()));
		flusherr();
		status = BAD_STATUS;
#ifdef _DEBUG
		logerr(std::format("{}\n", e.what()));
		assert(false);
#endif
		throw;
	}
}

Value VM::runFunction(const PhsString &name, const Bytecode &bytecode, const bool &argsInit)
{
    isDirectCall = true;
    setup(bytecode, bytecode.functionEntries.find(name)->second);

	if (!argsInit) 
	{ 
		push(0);
	}

#ifdef _WIN32
	int out_fd = _fileno(stdout);
    int in_fd = _fileno(stdin);
	int old_out_mode = _setmode(out_fd, _O_BINARY);
    int old_in_mode = _setmode(in_fd, _O_BINARY);
#endif

    try 
	{
        evalLoop();
    } catch (const VM::Halt &) {
#ifdef _WIN32
	_setmode(out_fd, old_out_mode);
    _setmode(in_fd, old_in_mode);
#endif
		if (isDirectCall)
		{
			Value ret = pop();
			if (ret.isInt())
			{
				status = static_cast<int>(ret.asInt());
			} else {
				status = 0;
			}
			reset(true, false, true);
			return ret;
		}
			throw std::runtime_error("Function call was not properly handled!");
	}
#ifdef _WIN32
	_setmode(out_fd, old_out_mode);
    _setmode(in_fd, old_in_mode);
#endif
	throw std::runtime_error("Function did not return properly!");
	status = BAD_STATUS;
	isError = true;
	return phsnull;
}

void VM::cleanup()
{
#ifdef TRACING
	log(std::format("{}\n", PHS_SRC_LOC()));
	flush();
#endif
	for (auto &i : registers)
	{
		i = Value();
	}
	for (auto &i : variables)
	{
		i = Value();
	}
	flush();
	flusherr();
	reset(true, true, true);
}

void VM::reset(const bool &resetStack, const bool &resetFunctions, const bool &resetVariables)
{
#ifdef TRACING
	log(std::format("Calling: {}\n", PHS_SRC_LOC()));
	flush();
#endif
	if (resetStack)
	{
		callStack.clear();
		tracelog.clear();
		stack_pool.release();
		stack = std::pmr::vector<Value>(&stack_pool);
	}
	if (resetFunctions)
	{
		nativeFunctions.clear();
	}
	if (resetVariables)
	{
		variables.clear();
	}
	pc = 0;
	status = 0;
	m_bytecode = nullptr;
	isDirectCall = false;
}

PhsString VM::getInformation()
{
	int         callStackTop = callStack.empty() ? -1 : callStack.back();
	PhsString info;

	if (!stack.empty())
	{
		info += PhsString(std::format("Stack Top: {:T}\n", peek()));
	}

	PhsString registersStr;
	int         regCount = 0;

	for (const auto &reg : registers)
	{
		if (reg.getType() != ValueType::Null)
		{
			registersStr += std::format("R{}: {:T}\n", regCount, reg);
		}
		regCount++;
	}

	info += std::format("VM INFORMATION:\n{}PC: {}\nCS: {}", registersStr, pc, callStackTop);

	return info;
}

PhsString VM::getBytecodeInformation()
{
	PhsString info;
	PhsString constants;
	PhsString variables;
	PhsString functions;
	PhsString instructions;

	for (const auto &constant : m_bytecode->constants)
	{
		constants += std::format("{:T}\n", constant);
	}
	for (const auto &variable : m_bytecode->variables)
	{
		variables += std::format("{}\n", variable.first);
	}
	for (const auto &function : m_bytecode->functionEntries)
	{
		functions += std::format("{}() PC = {}\n", function.first, function.second);
	}
#ifdef TRACING
	for (const auto &instruction : m_bytecode->instructions)
	{
		instructions += std::format("{}({}, {}, {})\n", opCodeToString(instruction.op), instruction.operand1,
		                            instruction.operand2, instruction.operand3);
	}
#endif

	info += std::format(
	    "BYTECODE INFORMATION:\n\nConstants: {}\n{}\nVariables: {}\n{}\nFunctions: {}\n{}\nInstructions: {}\n{}",
	    m_bytecode->constants.size(), constants, m_bytecode->variables.size(), variables,
	    m_bytecode->functionEntries.size(), functions, m_bytecode->instructions.size(), instructions);
	return info;
}

void VM::log(const Value &msg)
{
	std::string s = msg.toString();
	c_print_stdout(s.c_str(), (i64)s.length());
}

void VM::logerr(const Value &msg)
{
	std::string s = msg.toString();
	c_print_stderr(s.c_str(), (i64)s.length());
}

void VM::flush()
{
	fflush(stdout);
}

void VM::flusherr()
{
	fflush(stderr);
}
} // namespace Phasor
