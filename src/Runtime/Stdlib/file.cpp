#include <filesystem>
#include <utility>
#include <vector>
#include <fstream>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <phsint.hpp>

#include "StdLib.hpp"
#include "core/file_properties.h"
#include "core/pipe_streambuf.h"

namespace Phasor
{

namespace {
std::ios_base::openmode parseOpenMode(const PhsString &mode)
{
	if (mode == "r")  return std::ios::in;
	if (mode == "w")  return std::ios::out | std::ios::trunc;
	if (mode == "a")  return std::ios::out | std::ios::app;
	if (mode == "r+") return std::ios::in | std::ios::out;
	if (mode == "w+") return std::ios::in | std::ios::out | std::ios::trunc;
	if (mode == "a+") return std::ios::in | std::ios::out | std::ios::app;

	auto omode = (std::ios_base::openmode)0;
	if (mode.find('r') != std::string::npos) omode |= std::ios::in;
	if (mode.find('w') != std::string::npos) omode |= std::ios::out | std::ios::trunc;
	if (mode.find('a') != std::string::npos) omode |= std::ios::out | std::ios::app;
	if (mode.find('+') != std::string::npos)
	{
		omode |= std::ios::in | std::ios::out;
		if (mode.find('w') == std::string::npos) omode &= ~std::ios::trunc;
	}
	return omode;
}
} // namespace

void StdLib::registerFileFunctions(VM *vm)
{
	vm->registerNativeFunction("fopen", StdLib::file_open);
	vm->registerNativeFunction("fclose", StdLib::file_close);
	vm->registerNativeFunction("fabsolute", StdLib::file_absolute);
	vm->registerNativeFunction("frelative", StdLib::file_relative);
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
	vm->registerNativeFunction("ftouch", StdLib::file_create);
	vm->registerNativeFunction("fmkdir", StdLib::file_create_directory);
	vm->registerNativeFunction("frmdir", StdLib::file_remove_directory);
	vm->registerNativeFunction("freaddir", StdLib::file_read_directory);
	vm->registerNativeFunction("fjoin", StdLib::file_join_path);
	vm->registerNativeFunction("fsize", StdLib::file_get_size);
	vm->registerNativeFunction("fmemopen", StdLib::file_memory_open);
	vm->registerNativeFunction("fpipe", StdLib::file_pipe_open);
	vm->registerNativeFunction("fkind", StdLib::file_descriptor_kind);
}

Value StdLib::file_open(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "fopen");
	requireString(args[0], "fopen", "first argument (path)");
	requireString(args[1], "fopen", "second argument (mode)");
	PhsString path = args[0].string();
	auto omode = parseOpenMode(args[1].string());

	auto fs = std::make_unique<std::fstream>(path, omode);
	if (!fs->is_open()) return phsnull;
	return allocFileDescriptor(std::move(fs), StreamKind::File);
}

bool StdLib::file_close(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fclose");
	requireInt(args[0], "fclose", "argument (file descriptor)");
	i64 fd = args[0].asInt();
	auto& pool = getFilePool();
	if (fd >= 0 && std::cmp_less(fd, pool.size()) && pool[fd].stream)
	{
		if (pool[fd].kind == StreamKind::File)
			static_cast<std::fstream*>(pool[fd].stream.get())->close();
		pool[fd].stream.reset();
		pool[fd].kind = StreamKind::File;
		pool[fd].ownerProcess = -1;
		return true;
	}
	return false;
}

Value StdLib::file_memory_open(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fmemopen", true);
	if (args.size() > 2) PHS_ERROR("fmemopen() expects at most 2 arguments");

	requireString(args[0], "fmemopen", "first argument (mode)");
	auto omode = parseOpenMode(args[0].string());

	std::unique_ptr<std::stringstream> ss;
	if (args.size() == 2)
	{
		requireString(args[1], "fmemopen", "second argument (initial content)");
		ss = std::make_unique<std::stringstream>(args[1].stl_string(), omode);
	}
	else
	{
		ss = std::make_unique<std::stringstream>(omode);
	}
	return allocFileDescriptor(std::move(ss), StreamKind::Memory);
}

Value StdLib::file_pipe_open(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 0, "fpipe", true);

#if defined(_WIN32)
	HANDLE readHandle = nullptr, writeHandle = nullptr;
	SECURITY_ATTRIBUTES sa{ sizeof(sa), nullptr, TRUE };
	if (!CreatePipe(&readHandle, &writeHandle, &sa, 0)) return phsnull;
#else
	int fds[2];
	if (::pipe(fds) != 0) return phsnull;
	int readHandle = fds[0], writeHandle = fds[1];
#endif

	auto readStream  = std::make_unique<OwningIOStream>(std::make_unique<NativePipeStreamBuf>(readHandle, true));
	auto writeStream = std::make_unique<OwningIOStream>(std::make_unique<NativePipeStreamBuf>(writeHandle, false));

	i64 readFd  = allocFileDescriptor(std::move(readStream), StreamKind::Pipe);
	i64 writeFd = allocFileDescriptor(std::move(writeStream), StreamKind::Pipe);

	std::vector<Value> result{ readFd, writeFd };
	return Value::createArray(std::move(result));
}

PhsString StdLib::file_descriptor_kind(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fkind");
	requireInt(args[0], "fkind", "argument (file descriptor)");
	i64 fd = args[0].asInt();
	auto& pool = getFilePool();
	if (fd < 0 || std::cmp_greater_equal(fd, pool.size()) || !pool[fd].stream)
		PHS_ERROR("fkind() invalid file descriptor");
	switch (pool[fd].kind)
	{
		case StreamKind::File:   return "file";
		case StreamKind::Memory: return "memory";
		case StreamKind::Pipe:   return "pipe";
	}
	return "unknown";
}

PhsString StdLib::file_absolute(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fabsolute");
	requireString(args[0], "fabsolute", "argument (path)");
	return std::filesystem::weakly_canonical(std::filesystem::path(args[0].stl_string())).string();
}

PhsString StdLib::file_relative(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "frelative");
	requireString(args[0], "frelative", "argument (path)");

	std::filesystem::path path = std::filesystem::weakly_canonical(
		std::filesystem::path(args[0].stl_string())
	);

	std::filesystem::path cwd = std::filesystem::current_path();

	return std::filesystem::relative(path, cwd).string();
}

PhsString StdLib::file_stem(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fstem");
	requireString(args[0], "fstem", "argument (path)");
	return std::filesystem::path(args[0].stl_string()).stem().string();
}

PhsString StdLib::file_filename(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fname");
	requireString(args[0], "fname", "argument (path)");
	return std::filesystem::path(args[0].stl_string()).filename().string();
}

PhsString StdLib::file_extension(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fext");
	requireString(args[0], "fext", "argument (path)");
	return std::filesystem::path(args[0].stl_string()).extension().string();
}

PhsString StdLib::file_parent(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fparent");
	requireString(args[0], "fparent", "argument (path)");
	return std::filesystem::path(args[0].stl_string()).parent_path().string();
}

bool StdLib::file_is_directory(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fisdir");
	requireString(args[0], "fisdir", "argument (path)");
	return std::filesystem::is_directory(args[0].stl_string());
}

PhsString StdLib::file_join_path(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fjoin", true);

	auto appendComponent = [](std::filesystem::path &result, const Value &v, size_t argIndex)
	{
		if (v.isString())
		{
			result /= v.stl_string();
			return;
		}

		if (v.isArray())
		{
			auto arr = v.asArray();
			if (arr->empty())
				PHS_ERROR(std::string("fjoin() expects a non-empty array of strings for argument ")
				          + std::to_string(argIndex));

			for (size_t i = 0; i < arr->size(); ++i)
			{
				const Value &elem = (*arr)[i];
				if (!elem.isString())
					PHS_ERROR(std::string("fjoin() expects an array of strings for argument ")
					          + std::to_string(argIndex) + ", but element " + std::to_string(i)
					          + " is a " + Value::typeToString(elem.getType()).stl_string());

				result /= elem.stl_string();
			}
			return;
		}

		PHS_ERROR(std::string("fjoin() expects a string or an array of strings as argument ")
		          + std::to_string(argIndex) + ", but got a "
		          + Value::typeToString(v.getType()).stl_string());
	};

	std::filesystem::path result;

	if (args.size() == 1)
	{
		if (!args[0].isArray())
			PHS_ERROR(std::string("fjoin() with a single argument expects an array of strings"));

		appendComponent(result, args[0], 0);
	} else if (args.size() == 2) {
		appendComponent(result, args[0], 0);
		appendComponent(result, args[1], 1);
	} else {
		PHS_ERROR(std::string("fjoin() expects either a single array argument or two arguments (string or array)"));
	}

	return result.string();
}

i64 StdLib::file_get_size(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fsize");
	requireString(args[0], "fsize", "argument (path)");
	return std::filesystem::file_size(args[0].stl_string());
}

Value StdLib::file_read(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fread");

	if (args[0].isInt())
	{
		std::iostream* fs = getFileDescriptor(args[0].asInt());
		if (fs == nullptr) 
		{
			return phsnull;
		}
		std::stringstream buffer;
		buffer << fs->rdbuf();
		return buffer.str();
	}

	requireString(args[0], "fread", "argument (file descriptor or path)");

	std::filesystem::path path = args[0].stl_string();
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
		PHS_ERROR("freadln requires 1 or 2 arguments");
	}

	if (args.size() == 1)
	{
		if (!args[0].isInt()) PHS_ERROR("freadln with 1 arg requires an FD");
		std::iostream* fs = getFileDescriptor(args[0].asInt());
		if (fs == nullptr)
		{
			return phsnull;
		}
		
		std::string lineContent;
		if (std::getline(*fs, lineContent))
		{
			return lineContent;
		}
		return phsnull;
	}

	requireInt(args[1], "freadln", "second argument (line number)");
	i64 lineNum = args[1].asInt();
	std::istream* is = nullptr;
	std::ifstream tempFile;

	if (args[0].isInt())
	{
		std::iostream* fs = getFileDescriptor(args[0].asInt());
		if (fs == nullptr)
		{
			return phsnull;
		}
		fs->clear();
		fs->seekg(0, std::ios::beg);
		is = fs;
	} else {
		requireString(args[0], "freadln", "first argument (file descriptor or path)");
		std::filesystem::path path = args[0].stl_string();
		tempFile.open(path);
		if (!tempFile.is_open())
		{
			PHS_ERROR("Could not open file: " + path.string());
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
		PHS_ERROR("fwriteln modifying arbitrary lines isn't supported for file descriptors; use a file path instead.");
	}

	requireString(args[0], "fwriteln", "first argument (path)");
	requireInt(args[1], "fwriteln", "second argument (line number)");

	std::filesystem::path path = args[0].stl_string();
	i64               lineNum = args[1].asInt();
	PhsString           content = args[2].string();

	if (lineNum < 0)
		PHS_ERROR("fwriteln() line number cannot be negative");

	// Read all lines first
	std::ifstream inFile(path);
	if (!inFile.is_open())
	{
		PHS_ERROR("Could not open file for reading: " + path.string());
		return false;
	}

	std::vector<PhsString> lines;
	std::string              line;
	while (std::getline(inFile, line))
	{
		lines.emplace_back(line);
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
		PHS_ERROR("Could not open file for writing: " + path.string());
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
		std::iostream* fs = getFileDescriptor(args[0].asInt());
		if (fs == nullptr)
		{
			return false;
		}
		(*fs) << args[1].string();
		fs->flush();
		return true;
	}

	requireString(args[0], "fwrite", "first argument (file descriptor or path)");

	std::filesystem::path path = args[0].stl_string();
	std::ofstream         file(path);
	if (!file.is_open())
	{
		PHS_ERROR("Could not open file for writing: " + path.string());
		return false;
	}
	file << args[1].string();
	return true;
}

bool StdLib::file_exists(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fexists");
	requireString(args[0], "fexists", "argument (path)");
	return std::filesystem::exists(args[0].stl_string());
}

bool StdLib::file_append(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "fappend");

	if (args[0].isInt())
	{
		std::iostream* fs = getFileDescriptor(args[0].asInt());
		if (fs == nullptr)
		{
			return false;
		}
		fs->seekp(0, std::ios::end); // Safe jumping to EOF for sequential appends 
		(*fs) << args[1].string();
		fs->flush();
		return true;
	}

	requireString(args[0], "fappend", "first argument (file descriptor or path)");

	std::filesystem::path path = args[0].stl_string();
	std::ofstream         file(path, std::ios::app);
	if (!file.is_open())
	{
		PHS_ERROR("Could not open file for writing: " + path.string());
		return false;
	}
	file << args[1].string();
	return true;
}

bool StdLib::file_delete(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "frm");
	requireString(args[0], "frm", "argument (path)");
	std::filesystem::path path = args[0].stl_string();
	if (std::filesystem::exists(path))
	{
		return std::filesystem::remove(path);
	}
	return false;
}

bool StdLib::file_rename(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "frn");
	requireString(args[0], "frn", "first argument (source path)");
	requireString(args[1], "frn", "second argument (destination path)");
	std::filesystem::path src = args[0].stl_string();
	std::filesystem::path dest = args[1].stl_string();

	if (!std::filesystem::exists(src))
	{
		return false;
	}

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
	requireString(args[0], "fcd", "argument (path)");
	std::filesystem::path dest = args[0].stl_string();
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
	if (args.size() > 3)
		PHS_ERROR("fcp() expects at most 3 arguments");

	requireString(args[0], "fcp", "first argument (source path)");
	requireString(args[1], "fcp", "second argument (destination path)");

	bool overwrite = false;
	if (args.size() == 3)
	{
		requireBool(args[2], "fcp", "third argument (overwrite)");
		overwrite = args[2].asBool();
	}
	std::filesystem::path src = args[0].stl_string();
	std::filesystem::path dest = args[1].stl_string();

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
	requireString(args[0], "fmv", "first argument (source path)");
	requireString(args[1], "fmv", "second argument (destination path)");
	std::filesystem::path src = args[0].stl_string();
	std::filesystem::path dest = args[1].stl_string();
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
	requireString(args[0], "fpropedit", "first argument (path)");
	requireString(args[1], "fpropedit", "second argument (property)");
	if (args[1].stl_string().empty())
		PHS_ERROR("fpropedit() property must be a non-empty string");
	if (!args[2].isInt())
		PHS_ERROR("fpropedit() expects an integer as its third argument (epoch)");
	if (args[2].asInt() < 0)
	{
		PHS_ERROR("epoch must be a non-negative integer");
	}
	std::filesystem::path path = args[0].stl_string();
	char                  param = args[1].stl_string()[0];
	i64               epoch = args[2].asInt();
	return PHASORstd_file_setProperties(const_cast<char *>(path.string().c_str()), param, epoch);
}

i64 StdLib::file_property_get(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "fpropget");
	requireString(args[0], "fpropget", "first argument (path)");
	requireString(args[1], "fpropget", "second argument (property)");
	if (args[1].string().empty())
		PHS_ERROR("fpropget() property must be a non-empty string");
	std::filesystem::path path = args[0].stl_string();
	char                  param = args[1].string()[0];
	return PHASORstd_file_getProperties(const_cast<char *>(path.string().c_str()), param);
}

bool StdLib::file_create(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fcreate");
	requireString(args[0], "fcreate", "argument (path)");
	std::filesystem::path path = args[0].stl_string();
	std::ofstream         file(path);
	if (!file.is_open())
	{
		PHS_ERROR("Could not open file: " + path.string());
		return false;
	}
	file.close();
	return true;
}

Value StdLib::file_read_directory(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "freaddir");
	requireString(args[0], "freaddir", "argument (path)");
	PhsString path = args[0].string();

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
	requireString(args[0], "fmkdir", "argument (path)");
	std::filesystem::path path = args[0].stl_string();
	if (std::filesystem::exists(path))
	{
		return false;
	}
	std::filesystem::create_directory(path);
	return true;
}

bool StdLib::file_remove_directory(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "frmdir");
	requireString(args[0], "frmdir", "first argument (path)");
	requireBool(args[1], "frmdir", "second argument (recursive)");
	std::filesystem::path path = args[0].stl_string();
	bool                  recursive = args[1].asBool();
	if (std::filesystem::exists(path))
	{
		if (recursive)
		{
			return std::filesystem::remove_all(path) > 0;
		} else {
			return std::filesystem::remove(path);
		}
	}
	return true;
}
} // namespace Phasor
