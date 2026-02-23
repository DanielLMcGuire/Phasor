#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include "../VM/VM.hpp"

namespace Phasor
{

struct ModuleCache
{
	std::filesystem::path           path;
	std::filesystem::file_time_type lastLoaded;
	InstanceHandle                  handle = NULL_HANDLE;
	InstanceHandle                  owner = NULL_HANDLE;
};

struct ModuleManifest
{
	std::string                        name;
	std::string                        entry;
	std::vector<std::filesystem::path> sources;
	std::vector<std::filesystem::path> imports;
	std::vector<std::string>           exports;
	std::filesystem::path              outDir;
	std::string                        version;
	bool                               lazy = false;
	std::vector<std::string>           checksums;

	bool hasEntry() const
	{
		return !entry.empty();
	}
};

class ModuleLoader
{
  public:
	explicit ModuleLoader(VM &vm);

	InstanceHandle loadModule(const std::filesystem::path &rawPath, InstanceHandle owner = NULL_HANDLE);
	
    InstanceHandle callTrans(InstanceHandle caller, InstanceHandle target, const std::string &functionName,
	                         const std::vector<Value> &args);
	
    InstanceHandle callExtern(InstanceHandle caller, const std::filesystem::path &rawPath,
	                          const std::string &functionName, const std::vector<Value> &args);

	ModuleManifest parseManifest(const std::filesystem::path &path);
	bool           validateChecksums(const ModuleManifest &manifest);

  private:
	VM &m_vm;
	std::map<std::pair<std::filesystem::path, InstanceHandle>, ModuleCache> m_moduleCache;

	ModuleCache *findCache(const std::filesystem::path &path, InstanceHandle owner);
	void         insertCache(ModuleCache entry);
	void         evictCache(const std::filesystem::path &path, InstanceHandle owner);
};

} // namespace Phasor