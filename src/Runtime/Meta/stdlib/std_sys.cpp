// PULSAR STANDARD LIBRARY IMPLEMENTATION -- stdlib/std_sys.cpp -- Standard System Operations
// Author: Daniel McGuire 

#include "stdlib.h"
#ifdef _WIN32
#include <windows.h>
#endif

void PULSARSTD_api::system::print_error(const std::string& message) {
    std::cerr << message << std::endl;
}

int PULSARSTD_api::system::execute(const std::string& command) {
    return std::system(command.c_str());
}

void PULSARSTD_api::system::wait_for_enter() {
    std::cout << "Press Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void PULSARSTD_api::system::clear_screen() {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD cellCount;
    DWORD count;
    COORD homeCoords = {0, 0};

    if (hConsole == INVALID_HANDLE_VALUE) return;

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) return;
    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    if (!FillConsoleOutputCharacter(hConsole, (TCHAR)' ', cellCount, homeCoords, &count)) return;
    if (!FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, homeCoords, &count)) return;

    SetConsoleCursorPosition(hConsole, homeCoords);
#else
    std::cout << "\033[2J\033[1;1H" << std::flush;
#endif
}

std::string PULSARSTD_api::system::get_input() {
    std::string input;
    std::getline(std::cin, input);
    return input;
}


    