#include <filesystem>
#include <vector>
#include <fstream>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <phsint.hpp>

#include "StdLib.hpp"
#include "core/file_properties.h"

namespace Phasor
{

namespace
{
	struct FileHandle
	{
		std::unique_ptr<std::fstream> stream;
	};

	static std::vector<FileHandle>& getFilePool()
	{
		static std::vector<FileHandle> pool;
		return pool;
	}

	static i64 allocFileDescriptor(std::unique_ptr<std::fstream> fs)
	{
		auto& pool = getFilePool();
		for (size_t i = 0; i < pool.size(); ++i)
		{
			if (!pool[i].stream)
			{
				pool[i].stream = std::move(fs);
				return static_cast<i64>(i);
			}
		}
		pool.push_back({std::move(fs)});
		return static_cast<i64>(pool.size() - 1);
	}

	static std::fstream* getFileDescriptor(i64 fd)
	{
		auto& pool = getFilePool();
		if (fd >= 0 && fd < static_cast<i64>(pool.size()) && pool[fd].stream)
		{
			return pool[fd].stream.get();
		}
		return nullptr;
	}
}

void StdLib::registerFileFunctions(VM *vm)
{
	vm->registerNativeFunction("fopen", StdLib::file_open);
	vm->registerNativeFunction("fclose", StdLib::file_close);
	
	vm->registerNativeFunction("fabsolute", StdLib::file_absolute);
	vm->registerNativeFunction("fstem", StdLib::file_stem);
	vm->registerNativeFunction("fname", StdLib::file_filename);
	vm->registerNativeFunction("fext", StdLib::file_extension);
	vm->registerNativeFunction("fparent", StdLib::file_parent);
	vm->registerNativeFunction("fisdir", StdLib::file_is_directory);
	vm->registerNativeFunction("fread", StdLib::file_read);
	vm->registerNativeFunction("fwrite", StdLib::file_write);
	vm->registerNativeFunction("fexists", StdLib::file_exists);
	vm->registerNativeFunction("freadln", StdLib::file_read_line);
	vm->registerNativeFunction("fwriteln", StdLib::file_write_line);
	vm->registerNativeFunction("fappend", StdLib::file_append);
	vm->registerNativeFunction("frm", StdLib::file_delete);
	vm->registerNativeFunction("frn", StdLib::file_rename);
	vm->registerNativeFunction("fcd", StdLib::file_current_directory);
	vm->registerNativeFunction("fcp", StdLib::file_copy);
	vm->registerNativeFunction("fmv", StdLib::file_move);
	vm->registerNativeFunction("fpropset", StdLib::file_property_edit);
	vm->registerNativeFunction("fpropget", StdLib::file_property_get);
	vm->registerNativeFunction("fmk", StdLib::file_create);
	vm->registerNativeFunction("fmkdir", StdLib::file_create_directory);
	vm->registerNativeFunction("frmdir", StdLib::file_remove_directory);
	vm->registerNativeFunction("freaddir", StdLib::file_read_directory);
	vm->registerNativeFunction("fjoin", StdLib::file_join_path);
	vm->registerNativeFunction("fsize", StdLib::file_get_size);
}

Value StdLib::file_open(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "fopen");
	std::string path = args[0].string();
	std::string mode = args[1].string();

	std::ios_base::openmode omode = (std::ios_base::openmode)0;
	
	if (mode == "r") omode = std::ios::in;
	else if (mode == "w") omode = std::ios::out | std::ios::trunc;
	else if (mode == "a") omode = std::ios::out | std::ios::app;
	else if (mode == "r+") omode = std::ios::in | std::ios::out;
	else if (mode == "w+") omode = std::ios::in | std::ios::out | std::ios::trunc;
	else if (mode == "a+") omode = std::ios::in | std::ios::out | std::ios::app;
	else 
	{
		if (mode.find('r') != std::string::npos) omode |= std::ios::in;
		if (mode.find('w') != std::string::npos) omode |= std::ios::out | std::ios::trunc;
		if (mode.find('a') != std::string::npos) omode |= std::ios::out | std::ios::app;
		if (mode.find('+') != std::string::npos) {
			omode |= std::ios::in | std::ios::out;
			if (mode.find('w') == std::string::npos) omode &= ~std::ios::trunc;
		}
	}

	auto fs = std::make_unique<std::fstream>(path, omode);
	if (!fs->is_open())
	{
		return phsnull;
	}
	return allocFileDescriptor(std::move(fs));
}

bool StdLib::file_close(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fclose");
	if (!args[0].isInt()) return false;
	i64 fd = args[0].asInt();
	auto& pool = getFilePool();
	if (fd >= 0 && fd < static_cast<i64>(pool.size()) && pool[fd].stream)
	{
		pool[fd].stream->close();
		pool[fd].stream.reset();
		return true;
	}
	return false;
}

PhsString StdLib::file_absolute(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fabsolute");
	return std::filesystem::weakly_canonical(std::filesystem::path(args[0].string())).string();
}

PhsString StdLib::file_stem(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fstem");
	return std::filesystem::path(args[0].string()).stem().string();
}

PhsString StdLib::file_filename(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fname");
	return std::filesystem::path(args[0].string()).filename().string();
}

PhsString StdLib::file_extension(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fext");
	return std::filesystem::path(args[0].string()).extension().string();
}

PhsString StdLib::file_parent(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fparent");
	return std::filesystem::path(args[0].string()).parent_path().string();
}

bool StdLib::file_is_directory(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fisdir");
	return std::filesystem::is_directory(args[0].string());
}

PhsString StdLib::file_join_path(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "fjoin");
	std::filesystem::path path1 = args[0].string();
	std::filesystem::path path2 = args[1].string();
	return (path1 / path2).string();
}

i64 StdLib::file_get_size(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fsize");
	return std::filesystem::file_size(args[0].string());
}

Value StdLib::file_read(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fread");

	if (args[0].isInt())
	{
		std::fstream* fs = getFileDescriptor(args[0].asInt());
		if (!fs) return phsnull;
		std::stringstream buffer;
		buffer << fs->rdbuf();
		return buffer.str();
	}

	std::filesystem::path path = args[0].string();
	std::ifstream         file(path);
	if (!file.is_open())
	{
		return phsnull; // Return null if file cannot be opened
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

Value StdLib::file_read_line(const std::vector<Value> &args, VM *)
{
	if (args.empty() || args.size() > 2)
	{
		throw std::runtime_error("freadln requires 1 or 2 arguments");
	}

	if (args.size() == 1)
	{
		if (!args[0].isInt()) throw std::runtime_error("freadln with 1 arg requires an FD");
		std::fstream* fs = getFileDescriptor(args[0].asInt());
		if (!fs) return phsnull;
		
		std::string lineContent;
		if (std::getline(*fs, lineContent))
			return lineContent;
		return phsnull;
	}

	i64 lineNum = args[1].asInt();
	std::istream* is = nullptr;
	std::ifstream tempFile;

	if (args[0].isInt())
	{
		std::fstream* fs = getFileDescriptor(args[0].asInt());
		if (!fs) return phsnull;
		fs->clear();
		fs->seekg(0, std::ios::beg);
		is = fs;
	}
	else
	{
		std::filesystem::path path = args[0].string();
		tempFile.open(path);
		if (!tempFile.is_open())
		{
			throw std::runtime_error("Could not open file: " + path.string());
		}
		is = &tempFile;
	}

	std::string lineContent;
	int         currentLine = 0;
	while (std::getline(*is, lineContent) && currentLine < lineNum)
	{
		currentLine++;
	}
	return lineContent;
}

bool StdLib::file_write_line(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 3, "fwriteln");

	if (args[0].isInt())
	{
		throw std::runtime_error("fwriteln modifying arbitrary lines isn't supported for file descriptors; use a file path instead.");
	}

	std::filesystem::path path = args[0].string();
	i64               lineNum = args[1].asInt();
	PhsString           content = args[2].string();

	// Read all lines first
	std::ifstream inFile(path);
	if (!inFile.is_open())
	{
		throw std::runtime_error("Could not open file for reading: " + path.string());
		return false;
	}

	std::vector<PhsString> lines;
	std::string              line;
	while (std::getline(inFile, line))
	{
		lines.push_back(line);
	}
	inFile.close();

	// Ensure we have enough lines
	while (lines.size() <= static_cast<size_t>(lineNum))
	{
		lines.emplace_back("");
	}

	// Update the line
	lines[lineNum] = content;

	// Write back to file
	std::ofstream outFile(path);
	if (!outFile.is_open())
	{
		throw std::runtime_error("Could not open file for writing: " + path.string());
		return false;
	}

	for (size_t i = 0; i < lines.size(); ++i)
	{
		outFile << lines[i];
		if (i != lines.size() - 1)
		{
			outFile << '\n';
		}
	}

	return true;
}

bool StdLib::file_write(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "fwrite");

	if (args[0].isInt())
	{
		std::fstream* fs = getFileDescriptor(args[0].asInt());
		if (!fs) return false;
		(*fs) << args[1].string();
		fs->flush();
		return true;
	}

	std::filesystem::path path = args[0].string();
	std::ofstream         file(path);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file for writing: " + path.string());
		return false;
	}
	file << args[1].string();
	return true;
}

bool StdLib::file_exists(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fexists");
	return std::filesystem::exists(args[0].string());
}

bool StdLib::file_append(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "fappend");

	if (args[0].isInt())
	{
		std::fstream* fs = getFileDescriptor(args[0].asInt());
		if (!fs) return false;
		fs->seekp(0, std::ios::end); // Safe jumping to EOF for sequential appends 
		(*fs) << args[1].string();
		fs->flush();
		return true;
	}

	std::filesystem::path path = args[0].string();
	std::ofstream         file(path, std::ios::app);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file for writing: " + path.string());
		return false;
	}
	file << args[1].string();
	return true;
}

bool StdLib::file_delete(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "frm");
	std::filesystem::path path = args[0].string();
	if (std::filesystem::exists(path))
	{
		return std::filesystem::remove(path);
	}
	return false;
}

bool StdLib::file_rename(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "frn");
	std::filesystem::path src = args[0].string();
	std::filesystem::path dest = args[1].string();

	if (!std::filesystem::exists(src))
		return false;

	std::error_code ec;
	std::filesystem::rename(src, dest, ec);
	return !ec;
}

Value StdLib::file_current_directory(const std::vector<Value> &args, VM *)
{
	// If no arguments, return current directory
	if (args.empty())
	{
		return std::filesystem::current_path().string();
	}
	checkArgCount(args, 1, "fcd");
	std::filesystem::path dest = args[0].string();
	if (std::filesystem::exists(dest) && std::filesystem::is_directory(dest))
	{
		std::filesystem::current_path(dest);
		return std::filesystem::current_path().string();
	}

	return false;
}

bool StdLib::file_copy(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 2, "fcp", true);
	bool overwrite = false;
	if (args.size() <= 3 && args.size() >= 2)
	{
		overwrite = args[2].asBool();
	}
	std::filesystem::path src = args[0].string();
	std::filesystem::path dest = args[1].string();

	if (!std::filesystem::exists(src))
	{
		vm->logerr("Source file doesn't exist.");
		vm->flusherr();
		return false;
	}

	if (std::filesystem::exists(dest) && !overwrite)
	{
		vm->logerr("Destination file already exists.");
		vm->flusherr();
		return false;
	}

	std::ifstream source(src, std::ios::binary | std::ios::in);
	if (!source.is_open())
	{
		vm->logerr("Failed to open source file.");
		vm->flusherr();
		return false;
	}

	std::ofstream destination(dest, std::ios::binary | std::ios::out | std::ios::trunc);
	if (!destination.is_open())
	{
		vm->logerr("Failed to open destination file.");
		vm->flusherr();
		return false;
	}

	destination << source.rdbuf();

	if (source.fail() || destination.fail())
	{
		vm->logerr("Error during file copy.");
		vm->flusherr();
		return false;
	}

	return true;
}

bool StdLib::file_move(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 2, "fmv");
	std::filesystem::path src = args[0].string();
	std::filesystem::path dest = args[1].string();
	bool                  status;
	status = std::filesystem::copy_file(src, dest);
	if (!status)
	{
		vm->logerr("Failed to copy file during move.");
		vm->flusherr();
		return false;
	}
	status = std::filesystem::remove(src);
	return status;
}

bool StdLib::file_property_edit(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 3, "fpropedit");
	if (args[2].isInt() && args[2].asInt() < 0)
	{
		throw std::runtime_error("epoch must be a non-negative integer");
	}
	std::filesystem::path path = args[0].string();
	char                  param = args[1].string()[0];
	i64               epoch = args[2].asInt();
	return PHASORstd_file_setProperties(const_cast<char *>(path.string().c_str()), param, epoch);
}

i64 StdLib::file_property_get(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "fpropget");
	std::filesystem::path path = args[0].string();
	char                  param = args[1].string()[0];
	return PHASORstd_file_getProperties(const_cast<char *>(path.string().c_str()), param);
}

bool StdLib::file_create(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fcreate");
	std::filesystem::path path = args[0].string();
	std::ofstream         file(path);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file: " + path.string());
		return false;
	}
	file.close();
	return true;
}

Value StdLib::file_read_directory(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "freaddir");
	PhsString path = args[0].asString();

	std::vector<Value> entries;

	for (const auto &entry : std::filesystem::directory_iterator(path.str()))
	{
		entries.emplace_back(PhsString(entry.path().filename().string()));
	}

	return Value::createArray(std::move(entries));
}

bool StdLib::file_create_directory(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fmkdir");
	std::filesystem::path path = args[0].string();
	if (std::filesystem::exists(path))
		return false;
	std::filesystem::create_directory(path);
	return true;
}

bool StdLib::file_remove_directory(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "frmdir");
	std::filesystem::path path = args[0].string();
	bool                  recursive = args[1].asBool();
	if (std::filesystem::exists(path))
	{
		if (recursive)
		{
			if (std::filesystem::remove_all(path) > 0)
				return true;
			else
				return false;
		}
		else
		{
			return std::filesystem::remove(path);
		}
	}
	return true;
}
} // namespace Phasor