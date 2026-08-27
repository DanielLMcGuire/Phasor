#pragma once
#include <platform.h>
#include <version.h>
#include <PhasorString.hpp>
#include <unordered_map>
#include <vector>

/// @brief The Phasor Programming Language and Runtime
namespace Phasor
{

enum class DefineValueKind
{
	Number,
	String,
	Boolean,
};

struct DefineValue
{
	DefineValueKind kind = DefineValueKind::Number;
	Phasor::string     text;

	DefineValue() = default;
	DefineValue(DefineValueKind k, Phasor::string t) : kind(k), text(std::move(t))
	{
	}
};

using Defines = std::unordered_map<Phasor::string, DefineValue>;


inline void addDefaultDefines(Defines &defines, bool nativeTarget)
{
	auto setFlag = [&defines](const char *name, DefineValue value = DefineValue(DefineValueKind::Number, "1")) {
		defines[name] = std::move(value);
	};
	auto setNumber = [&setFlag](const char *name, long long value) {
		setFlag(name, DefineValue(DefineValueKind::Number, std::to_string(value)));
	};
	auto setString = [&setFlag](const char *name, Phasor::string value) {
		setFlag(name, DefineValue(DefineValueKind::String, std::move(value)));
	};
	auto setBool = [&setFlag](const char *name, bool value) {
		setFlag(name, DefineValue(DefineValueKind::Boolean, value ? "true" : "false"));
	};

	(void)setString;
	(void)setBool;

	if (nativeTarget)
	{
		setFlag("PHASOR_NATIVE");
	}

#if defined(PHS_IS_32)
	setNumber("PTR_SIZE", 4);
#else
	setNumber("PTR_SIZE", 8);
#endif

#if defined(TARGET_ARCH_ARM64)
	setFlag("ARM64");
	setNumber("COMP_ARCH", 0);
#elif defined(TARGET_ARCH_ARM)
	setFlag("ARM32");
	setNumber("COMP_ARCH", 1);
#elif defined(TARGET_ARCH_X64)
	setFlag("X86_64");
	setNumber("COMP_ARCH", 2);
#elif defined(TARGET_ARCH_X86)
	setFlag("X86");
	setNumber("COMP_ARCH", 3);
#else
	setNumber("COMP_ARCH", 4);
#endif

#if defined(_WIN32)
	setFlag("WIN32");
	setNumber("COMP_OS", 0);
#elif defined(__linux__)
	setFlag("LINUX");
	setNumber("COMP_OS", 1);
#elif defined(__APPLE__)
	setFlag("DARWIN");
	setNumber("COMP_OS", 2);
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(BSD)
	setFlag("BSD");
	setNumber("COMP_OS", 3);
#elif defined(__unix__)
	setFlag("UNIX");
	setNumber("COMP_OS", 4);
#else
	setNumber("COMP_OS", 5);
#endif

	setString("PHS_VERSION", PHASOR_VERSION_STRING);
	setString("PHS_VERSION_MAJOR", PHASOR_VERSION_MAJOR);
	setString("PHS_VERSION_MINOR", PHASOR_VERSION_MINOR);
	setString("PHS_VERSION_PATCH", PHASOR_VERSION_PATCH);

#if defined(_DEBUG)
	setFlag("DEBUG");
#endif

}

inline bool isNumericLiteralText(const Phasor::string &text)
{
	if (text.empty())
	{
		return false;
	}
	try
	{
		size_t idx = 0;
		(void)std::stod(text, &idx);
		return idx == text.size();
	}
	catch (...)
	{
		return false;
	}
}

inline DefineValue parseCliDefineValue(const Phasor::string &raw)
{
	if (raw.size() >= 2 &&
	    ((raw.front() == '"' && raw.back() == '"') || (raw.front() == '\'' && raw.back() == '\'')))
	{
		return DefineValue(DefineValueKind::String, raw.substr(1, raw.size() - 2));
	}

	if (raw == "true" || raw == "false")
	{
		return DefineValue(DefineValueKind::Boolean, raw);
	}

	if (isNumericLiteralText(raw))
	{
		return DefineValue(DefineValueKind::Number, raw);
	}

	return DefineValue(DefineValueKind::String, raw);
}

inline Defines resolveDefines(const std::vector<Phasor::string> &cliDefines, bool nativeTarget)
{
	Defines defines;
	addDefaultDefines(defines, nativeTarget);

	for (const auto &entry : cliDefines)
	{
		if (entry.empty())
		{
			continue;
		}
		auto eq = entry.find('=');
		if (eq == Phasor::string::npos)
		{
			defines[entry] = DefineValue(DefineValueKind::Number, "1");
		} else {
			Phasor::string name = entry.substr(0, eq);
			Phasor::string rawValue = entry.substr(eq + 1);
			defines[name] = parseCliDefineValue(rawValue);
		}
	}

	return defines;
}

} // namespace Phasor