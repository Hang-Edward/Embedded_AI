#include "Console.h"

#include <iostream>
#include <limits>
#include <windows.h>

void Console::info(const std::string& text) const {
    writeToStdout(text);
}

void Console::error(const std::string& text) const {
    writeToStderr(text);
}

std::string Console::askLine(const std::string& prompt) const {
    writeToStdout(prompt);
    std::string value;
    std::getline(std::cin, value);
    return value;
}

int Console::askInt(const std::string& prompt) const {
    writeToStdout(prompt);
    int value = 0;
    std::cin >> value;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

void Console::writeToStdout(const std::string& text) const {
    DWORD written = 0;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), text.data(), static_cast<DWORD>(text.size()), &written, nullptr);
}

void Console::writeToStderr(const std::string& text) const {
    DWORD written = 0;
    WriteFile(GetStdHandle(STD_ERROR_HANDLE), text.data(), static_cast<DWORD>(text.size()), &written, nullptr);
}
