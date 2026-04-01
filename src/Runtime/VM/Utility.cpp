#ifndef CMAKE
#include "VM.hpp"
#endif
#include <iostream>
#include <stdexcept>
#include <format>
#include <cassert>
#include "core/core.h"

namespace Phasor
{

int VM::run(const Bytecode &bc)
{
	m_bytecode = &bc;
	pc = 0;
	stack.clear();
	callStack.clear();

#ifdef TRACING
	log(std::format("\nVM::{}():\n\n{}\n", __func__, getBytecodeInformation()));
	flush();
#endif

	registers.fill(Value());
	variables.resize(m_bytecode->nextVarIndex);

	while (pc < m_bytecode->instructions.size())
	{
		const Instruction &instr = m_bytecode->instructions[pc++];
#ifdef TRACING
		log(std::format("\nVM::{}(): RUN (pc={})\n", __func__, pc - 1));
		flush();
#endif
		try
		{
			operation(instr.op, instr.operand1, instr.operand2, instr.operand3);
		}
		catch (const VM::Halt &)
		{
#ifdef TRACING
			log(std::format("\nVM::{}(): HALT (status={})\n\n{}\n\n", __func__, status, getInformation()));
			flush();
#endif
#ifdef _DEBUG
			assert(status == 0);
#endif
			return status;
		}
		catch (const std::exception &e)
		{
#ifdef TRACING
			logerr(std::format("\nVM::{}(): PANIC!\n\n{}\n{}\n\n", __func__, e.what(), getInformation()));
			flusherr();
#endif
			status = 1;
#ifdef _DEBUG
			logerr(std::format("{}\n", e.what()));
			assert(false);
#endif
			throw;
		}
	}
	return -1;
}

void VM::setImportHandler(const ImportHandler &handler)
{
#ifdef TRACING
	log(std::format("VM::{}()\n", __func__));
	flush();
#endif
	importHandler = handler;
}

void VM::cleanup() {
#ifdef TRACING
	log(std::format("VM::{}()\n", __func__));
	flush();
#endif
	for (auto &i : registers) { i = Value(); }
	for (auto &i : variables) { i = Value(); }
	flush();
	flusherr();
	reset(true, true, true);
}

void VM::reset(const bool &resetStack, const bool &resetFunctions, const bool &resetVariables)
{
#ifdef TRACING
	log(std::format("VM::{}()\n", __func__));
	flush();
#endif
	if (resetStack)
	{
		stack.clear();
		callStack.clear();
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
	m_bytecode = nullptr;
}

std::string VM::getInformation()
{
    int callStackTop = callStack.empty() ? -1 : callStack.back();
    std::string info;

    if (!stack.empty())
        info = std::format("Stack Top: {:T}\n", peek());

	std::string registersStr;
	int regCount = 0;

	for (const auto &reg : registers)
	{
		if (reg.getType() != ValueType::Null) {
			registersStr += std::format("R{}: {:T}\n", regCount, reg);
		}
		regCount++;
	}

    info += std::format(
        "VM INFORMATION:\n{}PC: {}\nCS: {}",
        registersStr,
        pc,
        callStackTop
    );

    return info;
}

std::string VM::getBytecodeInformation()
{
	std::string info;
	std::string constants;
	std::string variables;
	std::string functions;
	std::string instructions;

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

	info = std::format("BYTECODE INFORMATION:\n\nConstants: {}\n{}\nVariables: {}\n{}\nFunctions: {}\n{}\nInstructions: {}\n{}",
	m_bytecode->constants.size(), constants, 
	m_bytecode->variables.size(), variables, 
	m_bytecode->functionEntries.size(), functions,
	m_bytecode->instructions.size(), instructions);
	return info;
}

void VM::log(const Value &msg)
{
	std::string s = msg.toString();
	asm_print_stdout(s.c_str(), (int64_t)s.length());
}

void VM::logerr(const Value &msg)
{
	std::string s = msg.toString();
	asm_print_stderr(s.c_str(), (int64_t)s.length());
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
