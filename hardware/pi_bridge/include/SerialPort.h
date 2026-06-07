#pragma once

#include <cstdint>
#include <string>

class SerialPort {
public:
    SerialPort(std::string portName, std::uint32_t baudRate);
    ~SerialPort();

    SerialPort(const SerialPort&) = delete;
    SerialPort& operator=(const SerialPort&) = delete;

    bool open();
    void close();
    bool isOpen() const;

    bool writeLine(const std::string& text);
    std::string readAvailable(std::uint32_t waitMs);

    const std::string& portName() const;

private:
#if defined(_WIN32)
    std::string windowsDeviceName() const;
#endif

    std::string portName_;
    std::uint32_t baudRate_;
#if defined(_WIN32)
    void* handle_;
#else
    int fd_;
#endif
};
