#pragma once

#include <string>

class Console {
public:
    Console();

    void info(const std::string& text) const;
    void error(const std::string& text) const;
    std::string askLine(const std::string& prompt) const;
    int askInt(const std::string& prompt) const;

private:
    void writeToHandle(void* handle, const std::string& text) const;
};
