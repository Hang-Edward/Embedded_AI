#pragma once

#include <string>
#include <windows.h>

class SerialPort {
public:
    SerialPort(std::string portName, DWORD baudRate);
    ~SerialPort();

    SerialPort(const SerialPort&) = delete;
    SerialPort& operator=(const SerialPort&) = delete;

    bool open();
    void close();
    bool isOpen() const;

    bool writeLine(const std::string& text);
    std::string readAvailable(DWORD waitMs);

    const std::string& portName() const;

private:
    std::string windowsDeviceName() const;

    std::string portName_;
    DWORD baudRate_;
    HANDLE handle_;
};
