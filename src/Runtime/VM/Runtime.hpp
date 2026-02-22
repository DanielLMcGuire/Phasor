#pragma once
#include "../../Codegen/CodeGen.hpp"
#include "../Value.hpp"
#include <array>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <limits>

/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

class VM;

/// @brief Opaque index into Runtime::instances.
///        Never store a raw Instance* across a call boundary — always resolve
///        via Runtime. Handles remain stable as long as the instance is alive.
using InstanceHandle = std::size_t;

/// @brief Sentinel value for an invalid or unset handle
static constexpr InstanceHandle NULL_HANDLE = std::numeric_limits<InstanceHandle>::max();

enum class ErrorStatus
{
	Null,             ///< No error
	RuntimeError,     ///< General runtime fault
	StackUnderflow,   ///< Pop from empty stack
	InvalidOpcode,    ///< Unrecognised opcode
	ModuleNotFound,   ///< Import target could not be resolved
	ChecksumMismatch, ///< Source file failed checksum validation
	EntryNotFound,    ///< Named entry point does not exist in bytecode
	AccessViolation,  ///< Cross-instance access that was not declared as an import
};

/// @brief A single activation record on an instance's call stack.
///        Owns its own operand stack and register file.
struct Frame
{
	/// @brief Operand stack for this activation
	std::vector<Value> stack;

	/// @brief Program counter — index into the owning instance's bytecode instructions
	std::size_t pc = 0;

	/// @brief Register file — 32 general-purpose value registers
	///        ~1.1 KB per frame assuming sizeof(Value) == 36 bytes.
	std::array<Value, 32> registers;

	/// @brief Handle of the instance to return control to when this frame exits.
	///        NULL_HANDLE if this is the bottom frame of the instance.
	InstanceHandle returnToInstance = NULL_HANDLE;

	/// @brief PC to restore in the return instance's active frame on return.
	std::size_t returnPc = 0;

	Frame()
	{
		registers.fill(Value());
	}
};

/// @brief A running module. Each import that requires isolation gets its own Instance.
///        Owned exclusively by Runtime via unique_ptr — never heap-allocate directly.
struct Instance
{
	/// @brief Bytecode this instance is executing
	Bytecode code;

	/// @brief Call stack — back() is the active frame
	std::vector<Frame> callStack;

	/// @brief Variable storage indexed by variable slot
	std::vector<Value> variables;

	/// @brief Whether this instance is still executing.
	///        Set to false when HALT is reached or a fatal error occurs.
	bool alive = true;

	/// @brief Handles of modules this instance has loaded, indexed by import order.
	///        Used to enforce that cross-instance callbacks only target declared imports.
	std::vector<InstanceHandle> imports;

	/// @brief Error state — Null means no error
	ErrorStatus error = ErrorStatus::Null;

	/// @brief Human-readable error message, populated on fault
	std::string errorMsg;

	/// @brief Convenience: active frame reference. UB if callStack is empty — check first.
	Frame &activeFrame()
	{
		return callStack.back();
	}
	const Frame &activeFrame() const
	{
		return callStack.back();
	}

	/// @brief Push a new frame onto the call stack
	void pushFrame(InstanceHandle returnTo = NULL_HANDLE, std::size_t returnPc = 0)
	{
		Frame f;
		f.returnToInstance = returnTo;
		f.returnPc = returnPc;
		callStack.push_back(std::move(f));
	}

	/// @brief Pop the active frame and return it
	Frame popFrame()
	{
		Frame f = std::move(callStack.back());
		callStack.pop_back();
		return f;
	}
};

/// @brief One entry in the module cache.
///        The cache key is (canonical path, owner handle) — two instances that
///        both import the same module get separate cache entries and separate
///        Instance objects, preventing aliased mutable state.
struct ModuleCache
{
	/// @brief Canonical absolute path of the module's manifest (.json)
	std::filesystem::path path;

	/// @brief Timestamp at the time this entry was created, used for staleness checks
	std::filesystem::file_time_type lastLoaded;

	/// @brief Handle of the Instance that was created for this import
	InstanceHandle handle = NULL_HANDLE;

	/// @brief Handle of the Instance that requested this import.
	///        NULL_HANDLE for top-level (host-initiated) loads.
	InstanceHandle owner = NULL_HANDLE;
};

/// @brief Parsed representation of a module's .json manifest.
///        Produced by Runtime::parseManifest().
struct ModuleManifest
{
	/// @brief Module name (informational)
	std::string name;

	/// @brief Entry point in "x:y" format (x = source file stem, y = function name).
	///        Empty string means library mode — no top-level execution on load.
	std::string entry;

	/// @brief Source file paths relative to the manifest location
	std::vector<std::filesystem::path> sources;

	/// @brief Paths to manifests this module depends on
	std::vector<std::filesystem::path> imports;

	/// @brief Exported qualified names in "x:y" format
	std::vector<std::string> exports;

	/// @brief Version string (informational, not enforced at runtime)
	std::string version;

	/// @brief If true, do not load until the first call_trans / call_extern targeting this module
	bool lazy = false;

	/// @brief Per-source checksums, parallel to sources[].
	///        "SKIP" bypasses validation for that entry.
	std::vector<std::string> checksums;

	/// @brief Returns true if this manifest declares an entry point
	bool hasEntry() const
	{
		return !entry.empty();
	}
};

/// @brief Top-level execution environment.
///        Owns all instances and the module cache.
///        Entry point for loading and running Phasor programs.
struct Runtime
{
	/// @brief All live instances. Index == InstanceHandle.
	///        Slots are never reused during a Runtime's lifetime — destroyed instances
	///        have their unique_ptr reset to nullptr and alive set to false.
	std::vector<std::unique_ptr<Instance>> instances;

	/// @brief Module cache keyed by (canonical path, owner handle).
	///        Pair hash provided below.
	std::map<std::pair<std::filesystem::path, InstanceHandle>, ModuleCache> moduleCache;

	/// @brief Handle of the instance currently executing.
	///        Mutated by call_trans and call_extern opcodes.
	InstanceHandle current = NULL_HANDLE;

	/// @brief VM executor — shared across all instances in this runtime.
	///        VM holds no persistent execution state between run() calls.
	VM *vm = nullptr;

	/// @brief Allocate a new Instance and return its handle.
	///        The instance is empty — caller must populate code and push an initial frame.
	InstanceHandle createInstance()
	{
		InstanceHandle handle = instances.size();
		instances.push_back(std::make_unique<Instance>());
		return handle;
	}

	/// @brief Resolve a handle to a live Instance pointer.
	///        Returns nullptr if the handle is invalid or the instance has been destroyed.
	Instance *resolve(InstanceHandle handle)
	{
		if (handle == NULL_HANDLE || handle >= instances.size())
			return nullptr;
		return instances[handle].get();
	}

	/// @brief Mark an instance as dead and release its memory.
	///        Its handle becomes permanently invalid.
	void destroyInstance(InstanceHandle handle)
	{
		if (handle == NULL_HANDLE || handle >= instances.size())
			return;
		instances[handle].reset();
	}

	/// @brief Look up a cache entry by (path, owner). Returns nullptr on miss.
	ModuleCache *findCache(const std::filesystem::path &path, InstanceHandle owner)
	{
		auto it = moduleCache.find({path, owner});
		if (it == moduleCache.end())
			return nullptr;
		return &it->second;
	}

	/// @brief Insert or replace a cache entry.
	void insertCache(ModuleCache entry)
	{
		moduleCache[{entry.path, entry.owner}] = std::move(entry);
	}

	/// @brief Evict a cache entry and destroy its associated instance.
	void evictCache(const std::filesystem::path &path, InstanceHandle owner)
	{
		auto it = moduleCache.find({path, owner});
		if (it == moduleCache.end())
			return;
		destroyInstance(it->second.handle);
		moduleCache.erase(it);
	}

	/// @brief Run an already-loaded instance to completion.
	///        Sets current, delegates to vm->run(), restores current on return.
	///        Marks the instance alive=false when done.
	/// @return Exit status from vm->run()
	int execute(InstanceHandle handle);

	/// @brief Create an instance from a pre-compiled Bytecode object.
	///        Does not execute — caller must call execute() when ready.
	/// @param owner Handle of the requesting instance, or NULL_HANDLE for top-level
	/// @return Handle of the new instance
	InstanceHandle load(const Bytecode &code, InstanceHandle owner = NULL_HANDLE);

	/// @brief Load a module from a manifest path, respecting the cache.
	///        Creates a fresh instance per (path, owner) pair.
	///        Stale cache entries are evicted and reloaded automatically.
	/// @param rawPath Path to the module manifest (.json)
	/// @param owner   Handle of the requesting instance, or NULL_HANDLE for top-level
	/// @return Handle of the loaded instance
	InstanceHandle loadModule(const std::filesystem::path &rawPath, InstanceHandle owner = NULL_HANDLE);

	/// @brief Call a function in an already-loaded instance (cross-instance call).
	///        Validates that target is in caller's declared imports — throws AccessViolation if not.
	///        Pushes a return frame onto target, executes, pops frame, returns result on caller's stack.
	/// @return Handle of the target instance
	InstanceHandle callTrans(InstanceHandle caller, InstanceHandle target, const std::string &functionName,
	                         const std::vector<Value> &args);

	/// @brief Load a module by path and immediately call a function in it.
	///        Combines loadModule + callTrans. Creates a fresh instance if re-entrant.
	/// @return Handle of the target instance
	InstanceHandle callExtern(InstanceHandle caller, const std::filesystem::path &rawPath,
	                          const std::string &functionName, const std::vector<Value> &args);

	/// @brief Parse a module manifest JSON file into a ModuleManifest struct.
	ModuleManifest parseManifest(const std::filesystem::path &path);

	/// @brief Validate source file checksums declared in a manifest.
	///        Entries with value "SKIP" are bypassed.
	/// @return true if all non-skipped checksums match
	bool validateChecksums(const ModuleManifest &manifest);

	/// @brief Return a human-readable summary of all instances and cache entries
	std::string getInformation() const;
};

} // namespace Phasor