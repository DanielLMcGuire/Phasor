#include "Runtime.hpp"
#include "VM.hpp"
#include <iostream>
#include <stdexcept>
#include <fstream>

namespace Phasor
{

/// @brief Resolve and canonicalise a manifest path.
///        Returns an empty path if the file does not exist.
static std::filesystem::path resolvePath(const std::filesystem::path &raw)
{
	std::error_code ec;
	auto            canonical = std::filesystem::canonical(raw, ec);
	if (ec)
		return {};
	return canonical;
}

/// @brief Check whether a cache entry is stale by comparing the manifest's
///        last-write-time against the recorded timestamp.
static bool isCacheStale(const ModuleCache &entry)
{
	std::error_code ec;
	auto            current = std::filesystem::last_write_time(entry.path, ec);
	if (ec)
		return true;
	return current != entry.lastLoaded;
}

int Runtime::execute(InstanceHandle handle)
{
	if (!vm)
		throw std::runtime_error("Runtime has no VM attached");

	Instance *inst = resolve(handle);
	if (!inst)
		throw std::runtime_error("Runtime::execute — invalid handle " + std::to_string(handle));
	if (!inst->alive)
		throw std::runtime_error("Runtime::execute — instance " + std::to_string(handle) + " is no longer alive");

	InstanceHandle previous = current;
	current = handle;

	int result = 0;
	try
	{
		result = vm->run(*inst);
	}
	catch (const std::exception &ex)
	{
		inst->alive = false;
		inst->error = ErrorStatus::RuntimeError;
		inst->errorMsg = ex.what();
		current = previous;
		throw;
	}

	inst->alive = false;
	current = previous;
	return result;
}

InstanceHandle Runtime::load(const Bytecode &code, InstanceHandle owner)
{
	InstanceHandle handle = createInstance();
	Instance      *inst = resolve(handle);

	inst->code = code;

	// Record the owner so the cache key (path, owner) is correct for any
	// sub-imports this instance triggers
	// (owner is stored on the cache entry, not on Instance directly, but we
	//  thread it through so callers can pass it into insertCache)
	(void)owner; // used by caller when building the ModuleCache entry

	return handle;
}

InstanceHandle Runtime::loadModule(const std::filesystem::path &rawPath, InstanceHandle owner)
{
	// TODO: implement manifest parsing and bytecode compilation
	// Rough steps:
	//   1. resolvePath(rawPath) — error if empty
	//   2. Check findCache(canonical, owner) — if hit and !isCacheStale, return cached handle
	//   3. If stale, evictCache(canonical, owner)
	//   4. Read and parse the manifest JSON
	//   5. Validate checksums (skip entries marked "SKIP")
	//   6. Compile sources to Bytecode via CodeGenerator
	//   7. handle = load(bytecode, owner)
	//   8. Register imports declared in the manifest onto the new instance
	//   9. insertCache({ canonical, last_write_time, handle, owner })
	//  10. If manifest.lazy == false and entry != null, execute(handle)
	//  11. Return handle
	(void)rawPath;
	(void)owner;
	throw std::runtime_error("Runtime::loadModule — not yet implemented");
}

InstanceHandle Runtime::callTrans(InstanceHandle caller, InstanceHandle target, const std::string &functionName,
                                  const std::vector<Value> &args)
{
	// TODO: implement cross-instance function call
	// Rough steps:
	//   1. Validate that target is in caller's imports — AccessViolation if not
	//   2. Resolve target instance
	//   3. Look up functionName in target->code.functionEntries — EntryNotFound if missing
	//   4. Create a new frame on target with returnToInstance=caller, returnPc=caller active pc
	//   5. Copy args onto the new frame's stack
	//   6. Set target frame pc to the function entry index
	//   7. execute(target) — Runtime drives the loop, current switches to target
	//   8. On return, pop target frame, copy return value back to caller's stack
	//   9. Restore current to caller
	(void)caller;
	(void)target;
	(void)functionName;
	(void)args;
	throw std::runtime_error("Runtime::callTrans — not yet implemented");
}

InstanceHandle Runtime::callExtern(InstanceHandle caller, const std::filesystem::path &rawPath,
                                   const std::string &functionName, const std::vector<Value> &args)
{
	// TODO: implement external module call
	// Rough steps:
	//   1. loadModule(rawPath, caller) — respects cache, creates fresh instance if re-entrant
	//   2. Delegate to callTrans(caller, resolvedHandle, functionName, args)
	(void)caller;
	(void)rawPath;
	(void)functionName;
	(void)args;
	throw std::runtime_error("Runtime::callExtern — not yet implemented");
}

ModuleManifest Runtime::parseManifest(const std::filesystem::path &path)
{
	// TODO: implement JSON manifest parsing
	// Fields to extract:
	//   name        — string
	//   entry       — "x:y" where x matches x.phsb and y is the function name
	//                 null/absent means library mode (no top-level execution)
	//   sources     — array of source file paths relative to the manifest
	//   imports     — array of manifest paths this module depends on
	//   exports     — array of "x:y" qualified names
	//   version     — string
	//   lazy        — bool (default false)
	//   checksums   — array parallel to sources; "SKIP" bypasses validation for that entry
	(void)path;
	throw std::runtime_error("Runtime::parseManifest — not yet implemented");
}

bool Runtime::validateChecksums(const ModuleManifest &manifest)
{
	// TODO: implement per-source checksum validation
	// For each entry in manifest.checksums:
	//   - If value == "SKIP", continue
	//   - Otherwise, hash the corresponding source file (SHA-256 recommended)
	//     and compare against the stored value
	//   - Return false on first mismatch
	(void)manifest;
	throw std::runtime_error("Runtime::validateChecksums — not yet implemented");
}

std::string Runtime::getInformation() const
{
	std::string info;
	info += "Runtime: " + std::to_string(instances.size()) + " instance(s), ";
	info += std::to_string(moduleCache.size()) + " cache entrie(s)\n";
	info += "Current handle: ";
	info += (current == NULL_HANDLE ? "none" : std::to_string(current));
	info += "\n";

	for (std::size_t i = 0; i < instances.size(); ++i)
	{
		const auto &ptr = instances[i];
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

} // namespace Phasor