#include "VM.hpp"
#include "Runtime.hpp"
#include <iostream>
#include <stdexcept>
#include "core/core.h"

namespace Phasor
{

int VM::run(Instance &instance)
{
	m_instance = &instance;

	instance.callStack.clear();
	instance.variables.resize(instance.code.nextVarIndex);

	instance.pushFrame();

	while (instance.activeFrame().pc < instance.code.instructions.size())
	{
		const Instruction &instr = instance.code.instructions[instance.activeFrame().pc++];

#ifdef _DEBUG
		log(std::string("EXEC idx=" + std::to_string(instance.activeFrame().pc - 1) +
		                " op=" + std::to_string(static_cast<int>(instr.op)) +
		                " stack=" + std::to_string(instance.activeFrame().stack.size()) + "\n"));
		flush();
#endif

		try
		{
			operation(instr.op, instr.operand1, instr.operand2, instr.operand3, instr.operand4, instr.operand5);
		}
		catch (const VM::Halt &)
		{
			m_instance = nullptr;
			return status;
		}
		catch (const std::exception &ex)
		{
			std::cerr << ex.what() << "\n\n" << getInformation(instance) << "\n";
			m_instance = nullptr;
			throw;
		}
	}

	m_instance = nullptr;
	return 1;
}

std::string VM::getInformation(Instance &instance)
{
	if (instance.callStack.empty())
		return "No active frame";

	const Frame &frame = instance.activeFrame();
	std::string  info;

	info = "R0: " + frame.registers[0].toString();
	info += " | R1: " + frame.registers[1].toString();
	info += " | R2: " + frame.registers[2].toString();
	info += " | R3: " + frame.registers[3].toString();
	info += " | PC: " + std::to_string(frame.pc);
	info += " | Frame depth: " + std::to_string(instance.callStack.size());

	if (!frame.stack.empty())
		info += " | Stack top: " + frame.stack.back().toString();

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