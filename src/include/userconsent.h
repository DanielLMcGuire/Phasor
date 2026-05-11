#pragma once
#include <print>
#include <string>
#include <format>
#include <unordered_map>
#include <cstdlib>
#include <enum_array>

#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

// If you are here, you probably have one question, why?
// I cannot consistantly format grammar, this forces me (and potential contributors) to use "static time grammar"
// It reads *almost* like english so it works

/// @brief Priority level for the `prompt_consent` permission request
enum class EConsentVolition { 
	Might, 
	WouldLike, 
	Wants, 
	Needs, 
	HasTo, 
	Must ///< SHOULD ONLY BE USED IF IT WILL END THE PROGRAM ON FALSE
};

/// @brief Array for the `EConsentVolition` enum
static constexpr enum_array<EConsentVolition, std::string_view, 6> AConsentViolition = {{
    "might", 
	"would like to", 
	"wants to",
    "needs to", 
	"has to", 
	"must"
}};

/** @brief Prompt the user for consent
 * @param subsystem The string literal denoting the `subsystem` requesting consent
 * @param volition The priority level of the request
 * @param verb The string literal denoting the action being requested
 * @param noun The string literal denoting the object being acted upon
 * @param default_val The default value if the user doesn't respond
 * @return `true` if the user consents, `false` otherwise
 * 
 * The function will check if a TTY is present, if not, GUI fallbacks 
 * (e.g. WindowsAPI, AppleCF, zenity/kdialog) will be *attempted*, returning false on failure.
 */
template<size_t N1, size_t N2, size_t N3>
inline bool prompt_consent(const char (&subsystem)[N1], EConsentVolition volition, const char (&verb)[N2], const char (&noun)[N3], bool default_val = false) 
{
	bool res = false;
	std::string prompt = std::format("Phasor {} {} {} the {}. Is this okay?", subsystem, AConsentViolition[volition], verb, noun);
#ifdef _WIN32
	bool tty = _isatty(_fileno(stdin));
#else
	bool tty = isatty(fileno(stdin));
#endif

	if (!tty) { 
#ifdef _WIN32
		if (MessageBoxA(NULL, prompt.c_str(), "Phasor Programming Language", MB_YESNO) == IDYES) return true;
		else return false;
#elif __APPLE__
		CFStringRef cfTitle = CFStringCreateWithCString(NULL, "Phasor Programming Language", kCFStringEncodingUTF8);
    	CFStringRef cfMessage = CFStringCreateWithCString(NULL, prompt.c_str(), kCFStringEncodingUTF8);

		CFOptionFlags responseFlags;
		CFUserNotificationDisplayAlert(
			0,
			kCFUserNotificationNoteAlertLevel,
			nullptr,
			nullptr,
			nullptr,
			cfTitle,
			cfMessage,
			CFSTR("No"),
			CFSTR("Yes"),
			nullptr,
			&responseFlags
		);

		CFRelease(cfTitle);
		CFRelease(cfMessage);

		return responseFlags == kCFUserNotificationAlternateResponse;
#else
		bool has_display = std::getenv("DISPLAY") || std::getenv("WAYLAND_DISPLAY");
		if (!has_display) return false;

		const std::array<std::string, 2> tools = {"zenity", "kdialog"};

		for (const auto& tool : tools) {
			std::string cmd;
			if (tool == "zenity")
				cmd = std::format("zenity --question --text='{}' 2>/dev/null", prompt);
			else
				cmd = std::format("kdialog --yesno '{}' 2>/dev/null", prompt);

			int ret = std::system(cmd.c_str()); 
			if (ret == 127) continue;

			if (WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
				return true;
		}
#endif
	}
	std::println("");
	while (true) {
		std::string line;
		std::print("{} {} ", prompt, default_val ? "[Y/n]" : "[y/N]");
		std::getline(std::cin, line);
		if (line.empty()) return default_val;
		else if (line[0] == 'y' || line[0] == 'Y') res = true;
		else if (line[0] == 'n' || line[0] == 'N') res = false;
		else continue;
		break;
	}
	return res;
}