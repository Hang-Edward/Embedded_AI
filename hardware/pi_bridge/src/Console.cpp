#include "Console.h"

#include <iostream>
#include <limits>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace {

#if defined(_WIN32)
std::wstring utf8ToWide(const std::string& text) {
    if (text.empty()) {
        return {};
    }

    const int required = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), nullptr, 0);
    if (required <= 0) {
        return {};
    }

    std::wstring wide(static_cast<size_t>(required), L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, text.data(), static_cast<int>(text.size()), wide.data(), required);
    return wide;
}

bool isConsoleHandle(HANDLE handle) {
    DWORD mode = 0;
    return handle != INVALID_HANDLE_VALUE && handle != nullptr && GetConsoleMode(handle, &mode) != 0;
}
#endif

} // namespace

Console::Console() {
#if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

void Console::info(const std::string& text) const {
#if defined(_WIN32)
    writeToHandle(GetStdHandle(STD_OUTPUT_HANDLE), text);
#else
    std::cout << text;
    std::cout.flush();
#endif
}

void Console::error(const std::string& text) const {
#if defined(_WIN32)
    writeToHandle(GetStdHandle(STD_ERROR_HANDLE), text);
#else
    std::cerr << text;
    std::cerr.flush();
#endif
}

std::string Console::askLine(const std::string& prompt) const {
    info(prompt);
    std::string value;
    std::getline(std::cin, value);
    return value;
}

int Console::askInt(const std::string& prompt) const {
    info(prompt);
    int value = 0;
    std::cin >> value;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

void Console::writeToHandle(void* rawHandle, const std::string& text) const {
#if defined(_WIN32)
    HANDLE handle = static_cast<HANDLE>(rawHandle);
    if (isConsoleHandle(handle)) {
        const std::wstring wide = utf8ToWide(text);
        if (!wide.empty()) {
            DWORD written = 0;
            WriteConsoleW(handle, wide.data(), static_cast<DWORD>(wide.size()), &written, nullptr);
            return;
        }
    }

    DWORD written = 0;
    WriteFile(handle, text.data(), static_cast<DWORD>(text.size()), &written, nullptr);
#else
    (void)rawHandle;
    std::cout << text;
#endif
}
