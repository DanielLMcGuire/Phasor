#include "VM.hpp"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include "core/core.h"

#if defined(_MSC_VER)
#define COMPILE_MESSAGE(msg) __pragma(message(msg))
#elif defined(__GNUC__) || defined(__clang__)
#define DO_PRAGMA(x) _Pragma(#x)
#define COMPILE_MESSAGE(msg) DO_PRAGMA(message msg)
#else
#define COMPILE_MESSAGE(msg)
#endif
#define STR2(x) #x
#define STR(x) STR2(x)

namespace Phasor
{

InstanceHandle VM::createInstance()
{
	InstanceHandle handle = m_instances.size();
	m_instances.push_back(std::make_unique<Instance>());
	return handle;
}

InstanceHandle VM::load(const Bytecode &code, InstanceHandle owner)
{
	InstanceHandle handle = createInstance();
	Instance      *inst = resolve(handle);
	inst->code = code;
	(void)owner;
	return handle;
}

Instance *VM::resolve(InstanceHandle handle)
{
	if (handle == NULL_HANDLE || handle >= m_instances.size())
		return nullptr;
	return m_instances[handle].get();
}

void VM::destroyInstance(InstanceHandle handle)
{
	if (handle == NULL_HANDLE || handle >= m_instances.size())
		return;
	m_instances[handle].reset();
}

int VM::execute(InstanceHandle handle)
{
	Instance *inst = resolve(handle);
	if (!inst)
		throw std::runtime_error("VM::execute — invalid handle " + std::to_string(handle));
	if (!inst->alive)
		throw std::runtime_error("VM::execute — instance " + std::to_string(handle) + " is no longer alive");

	InstanceHandle previous = m_current;
	m_current = handle;

	int result = 0;
	try
	{
		result = run(*inst);
	}
	catch (const std::exception &ex)
	{
		inst->alive = false;
		inst->error = ErrorStatus::RuntimeError;
		inst->errorMsg = ex.what();
		m_current = previous;
		throw;
	}

	inst->alive = false;
	m_current = previous;
	return result;
}

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
			std::cerr << ex.what() << "\n\n" << getInformation() << "\n";
			m_instance = nullptr;
			throw;
		}
	}

	m_instance = nullptr;
	return 1;
}

std::string VM::getInformation()
{
	if (m_instance->callStack.empty())
		return "No active frame";

	const Frame &frame = m_instance->activeFrame();
	std::string  info;

	info = "R0: " + frame.registers[0].toString();
	info += " | R1: " + frame.registers[1].toString();
	info += " | R2: " + frame.registers[2].toString();
	info += " | R3: " + frame.registers[3].toString();
	info += " | PC: " + std::to_string(frame.pc);
	info += " | Frame depth: " + std::to_string(m_instance->callStack.size());

	if (!frame.stack.empty())
		info += " | Stack top: " + frame.stack.back().toString();

	return info;
}

std::string VM::getRuntimeInformation() const
{
	std::string info;
	info += "VM: " + std::to_string(m_instances.size()) + " instance(s), ";
	info += "Current handle: ";
	info += (m_current == NULL_HANDLE ? "none" : std::to_string(m_current));
	info += "\n";

	for (std::size_t i = 0; i < m_instances.size(); ++i)
	{
		const auto &ptr = m_instances[i];
		if (!ptr)
		{
			info += "  [" + std::to_string(i) + "] destroyed\n";
			continue;
		}
		info += "  [" + std::to_string(i) + "] alive=" + (ptr->alive ? "true" : "false");
		info += " frames=" + std::to_string(ptr->callStack.size());
		info += " vars=" + std::to_string(ptr->variables.size());
		if (ptr->error != ErrorStatus::Null)
			info += " error=\"" + ptr->errorMsg + "\"";
		info += "\n";
	}

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