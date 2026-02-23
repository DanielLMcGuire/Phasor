#include "VM.hpp"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <json.hpp>
#include "core/core.h"

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

ModuleCache *VM::findCache(const std::filesystem::path &path, InstanceHandle owner)
{
	auto it = m_moduleCache.find({path, owner});
	if (it == m_moduleCache.end())
		return nullptr;
	return &it->second;
}

void VM::insertCache(ModuleCache entry)
{
	m_moduleCache[{entry.path, entry.owner}] = std::move(entry);
}

void VM::evictCache(const std::filesystem::path &path, InstanceHandle owner)
{
	auto it = m_moduleCache.find({path, owner});
	if (it == m_moduleCache.end())
		return;
	destroyInstance(it->second.handle);
	m_moduleCache.erase(it);
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

// ─────────────────────────────────────────────
//  Module loading (stubs)
// ─────────────────────────────────────────────

InstanceHandle VM::loadModule(const std::filesystem::path &rawPath, InstanceHandle owner)
{
	// 1. Resolve canonical path
	std::error_code ec;
	auto            canonical = std::filesystem::canonical(rawPath, ec);
	if (ec || canonical.empty())
		throw std::runtime_error("VM::loadModule — cannot resolve path: " + rawPath.string());

	// 2. Check cache
	ModuleCache *cached = findCache(canonical, owner);
	if (cached)
	{
		// Check staleness
		auto currentTime = std::filesystem::last_write_time(canonical, ec);
		if (!ec && currentTime == cached->lastLoaded)
			return cached->handle; // Cache hit, not stale
		// Stale — evict and reload
		evictCache(canonical, owner);
	}

	// 3. Parse manifest
	ModuleManifest manifest = parseManifest(canonical);

	// 4. Validate checksums
	validateChecksums(manifest);

	// 5. Compile sources to Bytecode
	// For now this throws
	// THis should be moved out the VM
	throw std::runtime_error("VM::loadModule — source compilation not yet wired: " + canonical.string());

	/*
	Bytecode       bytecode;
	CodeGenerator  gen;
	for (const auto &src : manifest.sources)
	    bytecode = gen.generate(...); // wire your pipeline here

	// 6. Create instance
	InstanceHandle handle = load(bytecode, owner);
	Instance      *inst   = resolve(handle);

	// 7. Register declared imports onto the new instance
	for (const auto &importPath : manifest.imports)
	{
	    InstanceHandle importHandle = loadModule(importPath, handle);
	    inst->imports.push_back(importHandle);
	}

	// 8. Insert cache entry
	ModuleCache entry;
	entry.path       = canonical;
	entry.lastLoaded = std::filesystem::last_write_time(canonical);
	entry.handle     = handle;
	entry.owner      = owner;
	insertCache(std::move(entry));

	// 9. Execute immediately if not lazy and has an entry point
	if (!manifest.lazy && manifest.hasEntry())
	{
	    auto colon = manifest.entry.find(':');
	    if (colon != std::string::npos)
	    {
	        std::string funcName = manifest.entry.substr(colon + 1);
	        auto        it       = inst->code.functionEntries.find(funcName);
	        if (it == inst->code.functionEntries.end())
	            throw std::runtime_error("loadModule — entry point not found: " + funcName);
	        inst->activeFrame().pc = static_cast<std::size_t>(it->second);
	    }
	    execute(handle);
	}

	return handle;
	*/
}

InstanceHandle VM::callTrans(InstanceHandle caller, InstanceHandle target, const std::string &functionName,
                             const std::vector<Value> &args)
{
	Instance *callerInst = resolve(caller);
	if (!callerInst)
		throw std::runtime_error("callTrans — invalid caller handle");

	bool permitted =
	    std::find(callerInst->imports.begin(), callerInst->imports.end(), target) != callerInst->imports.end();
	if (!permitted)
		throw std::runtime_error("callTrans — access violation");

	Instance *targetInst = resolve(target);
	if (!targetInst)
		throw std::runtime_error("callTrans — invalid target handle");

	auto it = targetInst->code.functionEntries.find(functionName);
	if (it == targetInst->code.functionEntries.end())
		throw std::runtime_error("callTrans — entry not found: " + functionName);

	targetInst->alive = true;
	targetInst->pushFrame();
	for (const auto &arg : args)
		targetInst->activeFrame().stack.push_back(arg);
	targetInst->activeFrame().pc = static_cast<std::size_t>(it->second);

	execute(target);

	if (!targetInst->callStack.empty() && !targetInst->activeFrame().stack.empty())
	{
		Value retVal = targetInst->activeFrame().stack.back();
		targetInst->activeFrame().stack.pop_back();
		callerInst->activeFrame().stack.push_back(retVal);
	}

	return target;
}

InstanceHandle VM::callExtern(InstanceHandle caller, const std::filesystem::path &rawPath,
                              const std::string &functionName, const std::vector<Value> &args)
{
	InstanceHandle target = loadModule(rawPath, caller);
	return callTrans(caller, target, functionName, args);
}

ModuleManifest VM::parseManifest(const std::filesystem::path &path)
{
	std::ifstream f(path);
	if (!f.is_open())
		throw std::runtime_error("parseManifest — cannot open: " + path.string());

	nlohmann::json j;
	try
	{
		f >> j;
	}
	catch (const nlohmann::json::parse_error &ex)
	{
		throw std::runtime_error("parseManifest — JSON parse error in " + path.string() + ": " + ex.what());
	}

	ModuleManifest manifest;
	auto           dir = path.parent_path();

	manifest.name = j.value("name", "");
	manifest.entry = j.value("entry", "");
	manifest.version = j.value("version", "");
	manifest.lazy = j.value("lazy", false);

	if (j.contains("sources"))
		for (const auto &s : j["sources"])
			manifest.sources.push_back(dir / s.get<std::string>());

	if (j.contains("imports"))
		for (const auto &i : j["imports"])
			manifest.imports.push_back(std::filesystem::path(i.get<std::string>()));

	if (j.contains("exports"))
		for (const auto &e : j["exports"])
			manifest.exports.push_back(e.get<std::string>());

	if (j.contains("checksums"))
	{
		for (const auto &c : j["checksums"])
			manifest.checksums.push_back(c.get<std::string>());

		if (manifest.checksums.size() != manifest.sources.size())
			throw std::runtime_error("parseManifest — checksums length does not match sources length in " +
			                         path.string());
	}
	else
	{
		// No checksums provided — treat all as SKIP
		manifest.checksums.assign(manifest.sources.size(), "SKIP");
	}

	return manifest;
}

bool VM::validateChecksums(const ModuleManifest &manifest)
{
	// TODO: implement SHA-256 hashing
	for (std::size_t i = 0; i < manifest.checksums.size(); ++i)
	{
		if (manifest.checksums[i] == "SKIP")
			continue;
		throw std::runtime_error("validateChecksums — SHA-256 validation not yet implemented for: " +
		                         manifest.sources[i].string());
	}
	return true;
}

// ─────────────────────────────────────────────
//  Diagnostics
// ─────────────────────────────────────────────

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
	info += std::to_string(m_moduleCache.size()) + " cache entry/entries\n";
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