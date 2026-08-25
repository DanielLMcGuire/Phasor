#include "StdLib.hpp"
#include <Value.hpp>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

namespace Phasor
{

namespace
{

struct IniEntryInternal
{
	PhsString key;
	PhsString value;
};

struct IniSectionInternal
{
	PhsString                     name;
	std::vector<IniEntryInternal> entries;
};

using IniDataInternal = std::vector<IniSectionInternal>;

std::string trim(const std::string &s)
{
	size_t start = s.find_first_not_of(" \t\r\n");
	if (start == std::string::npos)
		return "";
	size_t end = s.find_last_not_of(" \t\r\n");
	return s.substr(start, end - start + 1);
}

std::string stripComment(const std::string &line)
{
	for (size_t i = 0; i < line.size(); ++i)
	{
		if ((line[i] == ';' || line[i] == '#') &&
		    (i == 0 || std::isspace(static_cast<unsigned char>(line[i - 1]))))
			return line.substr(0, i);
	}
	return line;
}

IniSectionInternal *findSection(IniDataInternal &data, const PhsString &name)
{
	for (auto &section : data)
	{
		if (section.name == name)
			return &section;
	}
	return nullptr;
}

IniEntryInternal *findEntry(IniSectionInternal &section, const PhsString &key)
{
	for (auto &entry : section.entries)
	{
		if (entry.key == key)
			return &entry;
	}
	return nullptr;
}

IniDataInternal parseIniString(const std::string &content)
{
	IniDataInternal data;

	std::istringstream in(content);
	IniSectionInternal *current = nullptr;
	std::string          line;

	while (std::getline(in, line))
	{
		std::string trimmed = trim(stripComment(line));
		if (trimmed.empty())
			continue;

		if (trimmed.front() == '[' && trimmed.back() == ']')
		{
			PhsString sectionName(trim(trimmed.substr(1, trimmed.size() - 2)));
			data.push_back({sectionName, {}});
			current = &data.back();
			continue;
		}

		size_t eq = trimmed.find('=');
		if (eq == std::string::npos)
			continue;

		PhsString key(trim(trimmed.substr(0, eq)));
		PhsString value(trim(trimmed.substr(eq + 1)));

		if (!current)
		{
			data.push_back({PhsString(""), {}});
			current = &data.back();
		}

		if (auto *existing = findEntry(*current, key))
			existing->value = value;
		else
			current->entries.push_back({key, value});
	}

	return data;
}

PhsString writeIniString(const IniDataInternal &data)
{
	std::string out;
	bool        firstSection = true;

	for (const auto &section : data)
	{
		if (!section.name.str().empty())
		{
			if (!firstSection)
				out += "\n";
			out += "[" + section.name.str() + "]\n";
		}
		firstSection = false;

		for (const auto &entry : section.entries)
			out += entry.key.str() + "=" + entry.value.str() + "\n";
	}

	return PhsString(out);
}

Value iniDataToValue(const IniDataInternal &data)
{
	Value root = Value::createStruct("IniFile");
	for (const auto &section : data)
	{
		Value sectionValue = Value::createStruct("IniSection");
		for (const auto &entry : section.entries)
			sectionValue.setField(entry.key, Value(entry.value));

		root.setField(section.name, sectionValue);
	}
	return root;
}

IniDataInternal valueToIniData(const Value &value)
{
	IniDataInternal data;
	if (!value.isStruct())
		return data;

	for (const auto &[sectionName, sectionValue] : value.asStruct()->fields)
	{
		IniSectionInternal section;
		section.name = sectionName;

		if (sectionValue.isStruct())
		{
			for (const auto &[key, val] : sectionValue.asStruct()->fields)
				section.entries.push_back({key, val.isString() ? val.string() : val.toString()});
		}

		data.push_back(std::move(section));
	}

	return data;
}

} // namespace

void StdLib::registerIniFunctions(VM *vm)
{
	vm->registerNativeFunction("ini_read", StdLib::ini_read);
	vm->registerNativeFunction("ini_write", StdLib::ini_write);
	vm->registerNativeFunction("ini_read_entry", StdLib::ini_read_entry);
	vm->registerNativeFunction("ini_write_entry", StdLib::ini_write_entry);
	vm->registerNativeFunction("ini_read_section", StdLib::ini_read_section);
	vm->registerNativeFunction("ini_write_section", StdLib::ini_write_section);
	vm->registerNativeFunction("ini_has_section", StdLib::ini_has_section);
	vm->registerNativeFunction("ini_has_entry", StdLib::ini_has_entry);
	vm->registerNativeFunction("ini_remove_section", StdLib::ini_remove_section);
	vm->registerNativeFunction("ini_remove_entry", StdLib::ini_remove_entry);
	vm->registerNativeFunction("ini_empty", StdLib::ini_empty);
}

Value StdLib::ini_read(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "ini_read");
	if (!args[0].isString())
		PHS_ERROR("ini_read() expects a string as its argument (ini content)");

	return iniDataToValue(parseIniString(args[0].stl_string()));
}

PhsString StdLib::ini_write(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "ini_write");
	if (!args[0].isStruct())
		PHS_ERROR("ini_write() expects a struct as its argument (ini data)");

	return writeIniString(valueToIniData(args[0]));
}

PhsString StdLib::ini_read_entry(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 3, "ini_read_entry");
	if (!args[0].isString())
		PHS_ERROR("ini_read_entry() expects a string as its first argument (ini content)");
	if (!args[1].isString())
		PHS_ERROR("ini_read_entry() expects a string as its second argument (section)");
	if (!args[2].isString())
		PHS_ERROR("ini_read_entry() expects a string as its third argument (key)");

	IniDataInternal      data    = parseIniString(args[0].stl_string());
	IniSectionInternal  *section = findSection(data, args[1].string());
	if (!section)
		return "";

	IniEntryInternal *entry = findEntry(*section, args[2].string());
	return entry ? entry->value : PhsString("");
}

PhsString StdLib::ini_write_entry(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 4, "ini_write_entry");
	if (!args[0].isString())
		PHS_ERROR("ini_write_entry() expects a string as its first argument (ini content)");
	if (!args[1].isString())
		PHS_ERROR("ini_write_entry() expects a string as its second argument (section)");
	if (!args[2].isString())
		PHS_ERROR("ini_write_entry() expects a string as its third argument (key)");

	IniDataInternal data = parseIniString(args[0].stl_string());

	PhsString            sectionName = args[1].string();
	IniSectionInternal  *section     = findSection(data, sectionName);
	if (!section)
	{
		data.push_back({sectionName, {}});
		section = &data.back();
	}

	PhsString key   = args[2].string();
	PhsString value = args[3].isString() ? args[3].string() : args[3].toString();

	if (auto *entry = findEntry(*section, key))
		entry->value = value;
	else
		section->entries.push_back({key, value});

	return writeIniString(data);
}

Value StdLib::ini_read_section(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "ini_read_section");
	if (!args[0].isString())
		PHS_ERROR("ini_read_section() expects a string as its first argument (ini content)");
	if (!args[1].isString())
		PHS_ERROR("ini_read_section() expects a string as its second argument (section)");

	IniDataInternal      data    = parseIniString(args[0].stl_string());
	IniSectionInternal  *section = findSection(data, args[1].string());
	if (!section)
		return phsnull;

	Value sectionValue = Value::createStruct("IniSection");
	for (const auto &entry : section->entries)
		sectionValue.setField(entry.key, Value(entry.value));

	return sectionValue;
}

PhsString StdLib::ini_write_section(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 3, "ini_write_section");
	if (!args[0].isString())
		PHS_ERROR("ini_write_section() expects a string as its first argument (ini content)");
	if (!args[1].isString())
		PHS_ERROR("ini_write_section() expects a string as its second argument (section)");
	if (!args[2].isStruct())
		PHS_ERROR("ini_write_section() expects a struct as its third argument (entries)");

	IniDataInternal data = parseIniString(args[0].stl_string());

	IniSectionInternal newSection;
	newSection.name = args[1].string();
	for (const auto &[key, val] : args[2].asStruct()->fields)
		newSection.entries.push_back({key, val.isString() ? val.string() : val.toString()});

	if (auto *existing = findSection(data, newSection.name))
		*existing = std::move(newSection);
	else
		data.push_back(std::move(newSection));

	return writeIniString(data);
}

bool StdLib::ini_has_section(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "ini_has_section");
	if (!args[0].isString())
		PHS_ERROR("ini_has_section() expects a string as its first argument (ini content)");
	if (!args[1].isString())
		PHS_ERROR("ini_has_section() expects a string as its second argument (section)");

	IniDataInternal data = parseIniString(args[0].stl_string());
	return findSection(data, args[1].string()) != nullptr;
}

bool StdLib::ini_has_entry(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 3, "ini_has_entry");
	if (!args[0].isString())
		PHS_ERROR("ini_has_entry() expects a string as its first argument (ini content)");
	if (!args[1].isString())
		PHS_ERROR("ini_has_entry() expects a string as its second argument (section)");
	if (!args[2].isString())
		PHS_ERROR("ini_has_entry() expects a string as its third argument (key)");

	IniDataInternal      data    = parseIniString(args[0].stl_string());
	IniSectionInternal  *section = findSection(data, args[1].string());
	if (!section)
		return false;

	return findEntry(*section, args[2].string()) != nullptr;
}

PhsString StdLib::ini_remove_section(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 2, "ini_remove_section");
	if (!args[0].isString())
		PHS_ERROR("ini_remove_section() expects a string as its first argument (ini content)");
	if (!args[1].isString())
		PHS_ERROR("ini_remove_section() expects a string as its second argument (section)");

	IniDataInternal data        = parseIniString(args[0].stl_string());
	PhsString       sectionName = args[1].string();

	auto it = std::find_if(data.begin(), data.end(),
	                        [&](const IniSectionInternal &s) { return s.name == sectionName; });
	if (it != data.end())
		data.erase(it);

	return writeIniString(data);
}

PhsString StdLib::ini_remove_entry(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 3, "ini_remove_entry");
	if (!args[0].isString())
		PHS_ERROR("ini_remove_entry() expects a string as its first argument (ini content)");
	if (!args[1].isString())
		PHS_ERROR("ini_remove_entry() expects a string as its second argument (section)");
	if (!args[2].isString())
		PHS_ERROR("ini_remove_entry() expects a string as its third argument (key)");

	IniDataInternal      data    = parseIniString(args[0].stl_string());
	IniSectionInternal  *section = findSection(data, args[1].string());

	if (section)
	{
		PhsString key = args[2].string();
		auto      it  = std::find_if(section->entries.begin(), section->entries.end(),
		                              [&](const IniEntryInternal &e) { return e.key == key; });
		if (it != section->entries.end())
			section->entries.erase(it);
	}

	return writeIniString(data);
}

bool StdLib::ini_empty(const Value::ArrayInstance &args, VM *)
{
	checkArgCount(args, 1, "ini_empty");
	if (!args[0].isString())
		PHS_ERROR("ini_empty() expects a string as its argument (ini content)");

	return trim(args[0].stl_string()).empty();
}

} // namespace Phasor