#include <filesystem>
#include <vector>

#include "StdLib.hpp"
#include "core/file_properties.h"

namespace Phasor
{

Value StdLib::registerFileFunctions(const std::vector<Value> &args, VM *vm)
{
	checkArgCount(args, 0, "include_stdfile");
	vm->registerNativeFunction("fabsolute", StdLib::file_absolute);
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
	vm->registerNativeFunction("fstat", StdLib::file_statistics);

	return true;
}

Value StdLib::file_absolute(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fabsolute");
	std::filesystem::path path = args[0].asString();
	std::filesystem::path fullPath = std::filesystem::weakly_canonical(path);
	return fullPath.string();
}

Value StdLib::file_read(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fread");
	std::filesystem::path path = args[0].asString();
	std::ifstream         file(path);
	if (!file.is_open())
	{
		return Value(); // Return null if file cannot be opened
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

Value StdLib::file_read_line(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "freadln");
	std::filesystem::path path = args[0].asString();
	int64_t               lineNum = args[1].asInt();
	std::ifstream         file(path);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file: " + path.string());
	}
	std::string lineContent;
	int         currentLine = 0;
	while (std::getline(file, lineContent) && currentLine < lineNum)
	{
		currentLine++;
	}
	return lineContent;
}

Value StdLib::file_write_line(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 3, "fwriteln");
	std::filesystem::path path = args[0].asString();
	int64_t               lineNum = args[1].asInt();
	std::string           content = args[2].asString();

	// Read all lines first
	std::ifstream inFile(path);
	if (!inFile.is_open())
	{
		throw std::runtime_error("Could not open file for reading: " + path.string());
	}

	std::vector<std::string> lines;
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

Value StdLib::file_write(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "fwrite");
	std::filesystem::path path = args[0].asString();
	std::ofstream         file(path);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file for writing: " + path.string());
	}
	file << args[1].asString();
	return true;
}

Value StdLib::file_exists(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fexists");
	return std::filesystem::exists(args[0].asString());
}

Value StdLib::file_append(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "fappend");
	std::filesystem::path path = args[0].asString();
	std::ofstream         file(path, std::ios::app);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file for writing: " + path.string());
	}
	file << args[1].asString();
	return true;
}

Value StdLib::file_delete(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "frm");
	std::filesystem::path path = args[0].asString();
	if (std::filesystem::exists(path))
	{
		std::filesystem::remove(path);
		return true;
	}
	return false;
}

Value StdLib::file_rename(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "frn");
	std::filesystem::path src = args[0].asString();
	std::string           dest = args[1].asString();
	if (std::filesystem::exists(src))
	{
		std::filesystem::rename(src, dest);
		return true;
	}
	return false;
}

Value StdLib::file_current_directory(const std::vector<Value> &args, VM *)
{
	// If no arguments, return current directory
	if (args.empty())
	{
		return std::filesystem::current_path().string();
	}
	checkArgCount(args, 1, "fcd");
	std::filesystem::path dest = args[0].asString();
	if (std::filesystem::exists(dest) && std::filesystem::is_directory(dest))
	{
		std::filesystem::current_path(dest);
		return std::filesystem::current_path().string();
	}

	return false;
}

Value StdLib::file_copy(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "fcp", true);
	bool overwrite = false;
	if (args.size() <= 3 && args.size() >= 2)
	{
		overwrite = args[2].asBool();
	}
	std::filesystem::path src = args[0].asString();
	std::filesystem::path dest = args[1].asString();

	if (!std::filesystem::exists(src))
	{
		std::cerr << "Source file doesn't exist." << std::endl;
		return false;
	}

	if (std::filesystem::exists(dest) && !overwrite)
	{
		std::cerr << "Destination file already exists." << std::endl;
		return false;
	}

	std::ifstream source(src, std::ios::binary | std::ios::in);
	if (!source.is_open())
	{
		std::cerr << "Failed to open source file." << std::endl;
		return false;
	}

	std::ofstream destination(dest, std::ios::binary | std::ios::out | std::ios::trunc);
	if (!destination.is_open())
	{
		std::cerr << "Failed to open destination file." << std::endl;
		return false;
	}

	destination << source.rdbuf();

	if (source.fail() || destination.fail())
	{
		std::cerr << "Error during file copy." << std::endl;
		return false;
	}

	return true;
}

Value StdLib::file_move(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "fmv");
	std::filesystem::path src = args[0].asString();
	std::filesystem::path dest = args[1].asString();
	bool status;
	status = std::filesystem::copy_file(src, dest);
	if (!status)
	{
		std::cerr << "Failed to copy file during move." << std::endl;
		return false;
	}
	status = std::filesystem::remove(src);
	return status;
}

Value StdLib::file_property_edit(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 3, "fpropedit");
	if (args[2].isInt() && args[2].asInt() < 0)
	{
		throw std::runtime_error("epoch must be a non-negative integer");
	}
	std::filesystem::path path = args[0].asString();
	char                  param = args[1].asString()[0];
	int64_t               epoch = args[2].asInt();
	return file_set_properties(const_cast<char *>(path.string().c_str()), param, epoch);
}

Value StdLib::file_property_get(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "fpropget");
	std::filesystem::path path = args[0].asString();
	char                  param = args[1].asString()[0];
	return file_get_properties(const_cast<char *>(path.string().c_str()), param);
}

Value StdLib::file_create(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fcreate");
	std::filesystem::path path = args[0].asString();
	std::ofstream         file(path);
	if (!file.is_open())
	{
		throw std::runtime_error("Could not open file: " + path.string());
	}
	file.close();
	return true;
}

Value StdLib::file_read_directory(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "freaddir");
	std::string path = args[0].asString();
	std::string result;
	try
	{
		for (const auto &entry : std::filesystem::directory_iterator(path))
		{
			if (!result.empty())
				result += "\n";
			result += entry.path().filename().string();
		}
		return result;
	}
	catch (const std::exception &e)
	{
		return Value(e.what()); // Return null on error
	}
}

Value StdLib::file_statistics(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fstat");
	std::string path = args[0].asString();
	uid_t       uid = 0;
	gid_t       gid = 0;
	nlink_t     nlink = file_get_links_count(path.c_str());
	file_get_owner_id(path.c_str(), &uid, &gid);
	try
	{
		auto status = std::filesystem::status(path);
		auto perms = status.permissions();

		Value::StructInstance stat;
		stat.structName = "FileStat";

		// Convert permissions to mode_t style
		int mode = 0;

		// Set file type bits
		if (std::filesystem::is_directory(status))
		{
			mode |= 0x4000; // Directory
		}
		else if (std::filesystem::is_regular_file(status))
		{
			mode |= 0x8000; // Regular file
		}
		else if (std::filesystem::is_symlink(status))
		{
			mode |= 0xA000; // Symbolic link
		}
		else
		{
			mode |= 0x8000; // Default to regular file
		}

		// Set permission bits
		mode |= ((perms & std::filesystem::perms::owner_read) != std::filesystem::perms::none) ? 0x100 : 0;
		mode |= ((perms & std::filesystem::perms::owner_write) != std::filesystem::perms::none) ? 0x80 : 0;
		mode |= ((perms & std::filesystem::perms::owner_exec) != std::filesystem::perms::none) ? 0x40 : 0;
		mode |= ((perms & std::filesystem::perms::group_read) != std::filesystem::perms::none) ? 0x20 : 0;
		mode |= ((perms & std::filesystem::perms::group_write) != std::filesystem::perms::none) ? 0x10 : 0;
		mode |= ((perms & std::filesystem::perms::group_exec) != std::filesystem::perms::none) ? 0x8 : 0;
		mode |= ((perms & std::filesystem::perms::others_read) != std::filesystem::perms::none) ? 0x4 : 0;
		mode |= ((perms & std::filesystem::perms::others_write) != std::filesystem::perms::none) ? 0x2 : 0;
		mode |= ((perms & std::filesystem::perms::others_exec) != std::filesystem::perms::none) ? 0x1 : 0;

		// Set file stats
		stat.fields["mode"] = Value(static_cast<int64_t>(mode));
		stat.fields["nlink"] = Value(static_cast<int64_t>(nlink));
		stat.fields["uid"] = Value(static_cast<int64_t>(uid));
		stat.fields["gid"] = Value(static_cast<int64_t>(gid));
		stat.fields["size"] = Value(static_cast<int64_t>(std::filesystem::file_size(path)));

		return Value(std::make_shared<Value::StructInstance>(std::move(stat)));
	}
	catch (const std::exception &e)
	{
		// Log the actual error for debugging
		std::cerr << "fstat error: " << e.what() << std::endl;
		return Value(); // Return null on error
	}
}

Value StdLib::file_create_directory(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 1, "fmkdir");
	std::filesystem::path path = args[0].asString();
	if (std::filesystem::exists(path))
		return false;
	std::filesystem::create_directory(path);
	return true;
}

Value StdLib::file_remove_directory(const std::vector<Value> &args, VM *)
{
	checkArgCount(args, 2, "frmdir");
	std::filesystem::path path = args[0].asString();
	bool recursive = args[1].asBool();	
	if (std::filesystem::exists(path))
	{
		if (recursive)
		{
			if (std::filesystem::remove_all(path) > 0) 
				return true;
			else
				return false;
		} else {
			return std::filesystem::remove(path);
		}
	}
	return true;
}
} // namespace Phasor