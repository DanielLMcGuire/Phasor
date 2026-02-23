#include "ModuleLoader.hpp"
#include "../../Codegen/CodeGen.hpp"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <json.hpp>

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

ModuleLoader::ModuleLoader(VM &vm) : m_vm(vm) {}

ModuleCache *ModuleLoader::findCache(const std::filesystem::path &path, InstanceHandle owner)
{
	auto it = m_moduleCache.find({path, owner});
	if (it == m_moduleCache.end())
		return nullptr;
	return &it->second;
}

void ModuleLoader::insertCache(ModuleCache entry)
{
	m_moduleCache[{entry.path, entry.owner}] = std::move(entry);
}

void ModuleLoader::evictCache(const std::filesystem::path &path, InstanceHandle owner)
{
	auto it = m_moduleCache.find({path, owner});
	if (it == m_moduleCache.end())
		return;
	m_vm.destroyInstance(it->second.handle);
	m_moduleCache.erase(it);
}

InstanceHandle ModuleLoader::loadModule(const std::filesystem::path &rawPath, InstanceHandle owner)
{
	COMPILE_MESSAGE("Warning: PHS_03 Modules have not been fully implemented! Line " STR(__LINE__))
	std::error_code ec;
	auto            canonical = std::filesystem::canonical(rawPath, ec);
	if (ec || canonical.empty())
		throw std::runtime_error("ModuleLoader::loadModule — cannot resolve path: " + rawPath.string());

	ModuleCache *cached = findCache(canonical, owner);
	if (cached)
	{
		auto currentTime = std::filesystem::last_write_time(canonical, ec);
		if (!ec && currentTime == cached->lastLoaded)
			return cached->handle;
		evictCache(canonical, owner);
	}

	ModuleManifest manifest = parseManifest(canonical);
	validateChecksums(manifest);

	// TODO: Check compiled or not and actually load bytecode

	throw std::runtime_error("ModuleLoader::loadModule — source compilation not yet wired: " + canonical.string());
}

InstanceHandle ModuleLoader::callTrans(InstanceHandle caller, InstanceHandle target, const std::string &functionName,
                                       const std::vector<Value> &args)
{
	COMPILE_MESSAGE("Warning: PHS_03 Modules have not been fully implemented! Line " STR(__LINE__))
	Instance *callerInst = m_vm.resolve(caller);
	if (!callerInst)
		throw std::runtime_error("callTrans — invalid caller handle");

	bool permitted = std::find(callerInst->imports.begin(), callerInst->imports.end(), target) != callerInst->imports.end();
	if (!permitted)
		throw std::runtime_error("callTrans — access violation");

	Instance *targetInst = m_vm.resolve(target);
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

	m_vm.execute(target);

	if (!targetInst->callStack.empty() && !targetInst->activeFrame().stack.empty())
	{
		Value retVal = targetInst->activeFrame().stack.back();
		targetInst->activeFrame().stack.pop_back();
		callerInst->activeFrame().stack.push_back(retVal);
	}

	return target;
}

InstanceHandle ModuleLoader::callExtern(InstanceHandle caller, const std::filesystem::path &rawPath,
                                        const std::string &functionName, const std::vector<Value> &args)
{
	COMPILE_MESSAGE("Warning: PHS_03 Modules have not been fully implemented! Line " STR(__LINE__))
	InstanceHandle target = loadModule(rawPath, caller);
	return callTrans(caller, target, functionName, args);
}

ModuleManifest ModuleLoader::parseManifest(const std::filesystem::path &path)
{
	COMPILE_MESSAGE("Warning: PHS_03 Modules have not been fully implemented! Line " STR(__LINE__))
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
	manifest.outDir = j.value("outDir", "out");

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
			throw std::runtime_error("parseManifest — checksums length does not match sources length in " + path.string());
	}
	else
	{
		manifest.checksums.assign(manifest.sources.size(), "SKIP");
	}

	return manifest;
}

bool ModuleLoader::validateChecksums(const ModuleManifest &manifest)
{
	COMPILE_MESSAGE("Warning: PHS_03 Modules have not been fully implemented! Line " STR(__LINE__))
	for (std::size_t i = 0; i < manifest.checksums.size(); ++i)
	{
		if (manifest.checksums[i] == "SKIP")
			continue;
		throw std::runtime_error("validateChecksums — SHA-256 validation not yet implemented for: " + manifest.sources[i].string());
	}
	return true;
}

} // namespace Phasor