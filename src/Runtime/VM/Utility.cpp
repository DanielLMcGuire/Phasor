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
	log(std::format("{}():\nProgram: Constants {}, Variables {}, Functions {}, Instructions: {}\n", __func__, m_bytecode->constants.size(),
	m_bytecode->variables.size(), m_bytecode->functionEntries.size(), m_bytecode->instructions.size()));
	flush();
#endif

	registers.fill(Value());
	variables.resize(m_bytecode->nextVarIndex);

	while (pc < m_bytecode->instructions.size())
	{
		const Instruction &instr = m_bytecode->instructions[pc++];
#ifdef TRACING
		log(std::format("\nDISPATCH: RUN (pc={})\n", pc - 1));
		flush();
#endif
		try
		{
			operation(instr.op, instr.operand1, instr.operand2, instr.operand3);
		}
		catch (const VM::Halt &)
		{
#ifdef TRACING
			log(std::format("\nDISPATCH: HALT (status={})\n{}", status, getInformation()));
			flush();
	#ifdef _DEBUG
			assert(status == 0);
	#endif
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
#ifdef TRACING
	log(std::format("{}()\n", __func__));
	flush();
#endif
	importHandler = handler;
}

void VM::cleanup() {
#ifdef TRACING
	log(std::format("{}()\n", __func__));
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
	log(std::format("{}()\n", __func__));
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

    info += std::format(
        "R0: {:T}\nR1: {:T}\nR2: {:T}\nR3: {:T}\nCurrent Program Counter: {}\nPC Stack Top: {}\n\n",
        registers[0],
        registers[1],
        registers[2],
        registers[3],
        pc,
        callStackTop
    );

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