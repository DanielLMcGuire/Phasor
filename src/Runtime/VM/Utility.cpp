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

	registers.fill(Value());
	variables.resize(m_bytecode->nextVarIndex);

	while (pc < m_bytecode->instructions.size())
	{
		const Instruction &instr = m_bytecode->instructions[pc++];
#ifdef _DEBUG
		log(std::string("EXEC idx=" + std::to_string(pc - 1) + " op=" + std::to_string(static_cast<int>(instr.op)) + " stack=" + std::to_string(stack.size()) + "\n"));
		flush();
#endif
		try
		{
			operation(instr.op, instr.operand1, instr.operand2, instr.operand3);
		}
		catch (const VM::Halt &)
		{
#ifdef _DEBUG
			log(getInformation());
			assert(status != 0);
#endif
			return status;
		}
		catch (const std::exception &)
		{
			std::cerr << getInformation() << "\n";
			throw;
		}
	}
	return 1;
}

void VM::setImportHandler(const ImportHandler &handler)
{
	importHandler = handler;
}

void VM::reset(const bool &resetStack, const bool &resetFunctions, const bool &resetVariables)
{
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
	int    callStackTop = callStack.empty() ? -1 : callStack.back();
	size_t programCounter = pc;
	std::string info;

	if (!stack.empty())
	{
		info += "Stack Top: ";
		info += peek().toString();
		info += '\n';
	}

	info += "R0: ";
	info += registers[0].toString();
	info += "\n R1: ";
	info += registers[1].toString();
	info += "\n R2: ";
	info += registers[2].toString();
	info += "\n R3: ";
	info += registers[3].toString();
	info += "\n Current Program Counter: " + std::to_string(programCounter);
	info += "\n PC Stack Top: " + std::to_string(callStackTop);
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