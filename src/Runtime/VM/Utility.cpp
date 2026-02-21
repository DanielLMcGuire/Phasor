#include "VM.hpp"
#include <iostream>
#include <stdexcept>
#include "core/core.h"

namespace Phasor
{

int VM::run(const Bytecode &bc)
{
	this->bytecode = &bc;
	pc = 0;
	stack.clear();
	callStack.clear();

	registers.fill(Value());
	variables.resize(bytecode->nextVarIndex);

	while (pc < bytecode->instructions.size())
	{
		const Instruction &instr = bytecode->instructions[pc++];
#ifdef _DEBUG
		log(std::string("EXEC idx=" + std::to_string(pc - 1) + " op=" + std::to_string(static_cast<int>(instr.op)) + " stack=" + std::to_string(stack.size()) + "\n"));
		flush();
#endif
		try
		{
			operation(instr.op, instr.operand1, instr.operand2, instr.operand3, instr.operand4, instr.operand5);
		}
		catch (const std::exception &ex)
		{
			std::cerr << ex.what() << "\n\n" << getInformation() << "\n";
			throw;
		}
	    catch (const VM::Halt &)
		{
			return status;
		}
	}
	return status;
}

void VM::setImportHandler(ImportHandler handler)
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
	bytecode = nullptr;
}

std::string VM::getInformation()
{
	int    callStackTop = callStack.empty() ? -1 : callStack.back();
	size_t programCounter = pc;

	std::string info = "Stack Top: ";
	info += peek().toString();
	info += " | R0: ";
	info += registers[0].toString();
	info += " | R1: ";
	info += registers[1].toString();
	info += " | R2: ";
	info += registers[2].toString();
	info += " | R3: ";
	info += registers[3].toString();
	info += " | Current Program Counter: " + std::to_string(programCounter);
	info += " | PC Stack Top: " + std::to_string(callStackTop);
	return info;
}

void VM::log(const Value &msg)
{
	std::string s = msg.toString();
	asm_print_stdout(s.c_str(), s.length());
}

void VM::logerr(const Value &msg)
{
	std::string s = msg.toString();
	asm_print_stderr(s.c_str(), s.length());
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