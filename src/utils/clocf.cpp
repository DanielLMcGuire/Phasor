#include <algorithm>
#include <charconv>
#include <format>
#include <iostream>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <optional>

#ifdef _WIN32
    #define NOMINMAX
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <io.h>
    #define popen _popen
    #define pclose _pclose
    #define STDIN_FILENO 0
#else
    #include <unistd.h>
    #include <sys/select.h>
    #include <sys/ioctl.h>
#endif

struct Language {
    std::string name;
    int code;
};

void print_help() {
    std::println("clocf - cloc formatter\n");
    std::println("Usage:");
    std::println("  cloc [options] <path> | clocf         Process cloc output");
    std::println("  clocf [options] <path>                Run cloc automatically");
    std::println("  clocf --help                          Show this help\n");
}

bool stdin_has_data() {
#ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    if (!GetConsoleMode(hStdin, &mode)) return true;
    DWORD events;
    if (GetNumberOfConsoleInputEvents(hStdin, &events) && events > 1) return true;
    return false;
#else
    timeval tv{.tv_sec = 0, .tv_usec = 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv) > 0 && FD_ISSET(STDIN_FILENO, &fds);
#endif
}

int get_console_width() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return static_cast<int>(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    }
    return 80;
#else
    winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        return static_cast<int>(w.ws_col);
    }
    return 80;
#endif
}

std::string read_line_dynamic(FILE* fp) {
    std::string line;
    int c;
    while ((c = fgetc(fp)) != EOF && c != '\n') {
        if (c != '\r') line += static_cast<char>(c);
    }
    return line;
}

std::optional<Language> parse_cloc_line(std::string_view line) {
    if (line.empty()) return std::nullopt;

    auto tokens = line 
        | std::views::split(' ') 
        | std::views::filter([](auto&& rng) { return !std::ranges::empty(rng); })
        | std::ranges::to<std::vector<std::string>>();

    if (tokens.size() < 5) return std::nullopt;

    for (size_t i = tokens.size() - 4; i < tokens.size(); ++i) {
        if (tokens[i].empty() || !std::ranges::all_of(tokens[i], [](char c) { return std::isdigit(static_cast<unsigned char>(c)); })) {
            return std::nullopt;
        }
    }

    int code = 0;
    std::from_chars(tokens.back().data(), tokens.back().data() + tokens.back().size(), code);

    std::string name;
    for (size_t i = 0; i < tokens.size() - 4; ++i) {
        if (i > 0) name += ' ';
        name += tokens[i];
    }

    return Language{name, code};
}

int main(int argc, char* argv[]) {
    std::vector<std::string_view> args(argv + 1, argv + argc);

    if (!args.empty() && (args[0] == "--help" || args[0] == "-h")) {
        print_help();
        return 0;
    }

    std::unique_ptr<FILE, decltype(&pclose)> pipe_input(nullptr, pclose);
    bool using_pipe = false;

    if (!args.empty()) {
        std::string cmd = "cloc";
        for (auto arg : args) {
            cmd += ' ';
            if (arg.find(' ') != std::string_view::npos) {
                cmd += std::format("\"{}\"", arg);
            } else {
                cmd += arg;
            }
        }
        
        FILE* raw_pipe = popen(cmd.c_str(), "r");
        if (!raw_pipe) {
            std::println(stderr, "Error: Failed to execute cloc");
            return 1;
        }
        pipe_input.reset(raw_pipe);
        using_pipe = true;
    } else if (!stdin_has_data()) {
        std::println(stderr, "Error: No input provided\n");
        print_help();
        return 1;
    }

    std::vector<Language> langs;
    int totalCode = 0;

    if (using_pipe) {
        while (true) {
            std::string line = read_line_dynamic(pipe_input.get());
            if (line.empty() && std::feof(pipe_input.get())) break;
            
            if (line.find("----") != std::string::npos || line.size() < 5) continue;
            if (line.find("Language") != std::string::npos || line.find("SUM") != std::string::npos) continue;

            if (auto lang = parse_cloc_line(line)) {
                totalCode += lang->code;
                langs.push_back(std::move(*lang));
            }
        }
    } else {
        std::string line;
        while (std::getline(std::cin, line)) {
            if (line.find("----") != std::string::npos || line.size() < 5) continue;
            if (line.find("Language") != std::string::npos || line.find("SUM") != std::string::npos) continue;

            if (auto lang = parse_cloc_line(line)) {
                totalCode += lang->code;
                langs.push_back(std::move(*lang));
            }
        }
    }

    std::ranges::sort(langs, std::greater{}, &Language::code);

    if (totalCode > 0) {
        int bar_width = std::max(10, (get_console_width() / 3) - 2);
        std::string bar;
        bar.reserve(bar_width);
        int cumulative_code = 0;

        for (size_t i = 0; i < langs.size(); ++i) {
            if (i < 6) {
                int old_pos = static_cast<int>((static_cast<double>(cumulative_code) / totalCode) * bar_width);
                cumulative_code += langs[i].code;
                int new_pos = static_cast<int>((static_cast<double>(cumulative_code) / totalCode) * bar_width);
                for (int c = 0; c < (new_pos - old_pos) && bar.size() < static_cast<size_t>(bar_width); ++c) {
                    bar += static_cast<char>('1' + i);
                }
            } else {
                cumulative_code += langs[i].code;
            }
        }
        while (bar.size() < static_cast<size_t>(bar_width)) bar += 'O';
        std::println("[{}]\n", bar);
    }

    size_t maxNameLen = 0;
    for (const auto& l : langs) maxNameLen = std::max(maxNameLen, l.name.size());

    int index_width = std::to_string(langs.size()).size();

    std::println("Total code lines: {}", totalCode);
    std::println("Language percentages:");

    for (size_t i = 0; i < langs.size(); ++i) {
        double percent = (totalCode > 0) ? (langs[i].code * 100.0 / totalCode) : 0.0;
        std::println("{:0>{}}: {:<{}} : {:6.2f}% ({} lines)", 
                     i + 1, index_width, 
                     langs[i].name, static_cast<int>(maxNameLen), 
                     percent, langs[i].code);
    }

    return 0;
}