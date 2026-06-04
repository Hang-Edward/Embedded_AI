#include "SerialPort.h"

#include <vector>

SerialPort::SerialPort(std::string portName, DWORD baudRate)
    : portName_(std::move(portName)), baudRate_(baudRate), handle_(INVALID_HANDLE_VALUE) {
}

SerialPort::~SerialPort() {
    close();
}

bool SerialPort::open() {
    if (isOpen()) {
        return true;
    }

    handle_ = CreateFileA(
        windowsDeviceName().c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );

    if (handle_ == INVALID_HANDLE_VALUE) {
        return false;
    }

    DCB dcb {};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(handle_, &dcb)) {
        close();
        return false;
    }

    dcb.BaudRate = baudRate_;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    dcb.fBinary = TRUE;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_ENABLE;

    if (!SetCommState(handle_, &dcb)) {
        close();
        return false;
    }

    COMMTIMEOUTS timeouts {};
    timeouts.ReadIntervalTimeout = 20;
    timeouts.ReadTotalTimeoutConstant = 80;
    timeouts.ReadTotalTimeoutMultiplier = 2;
    timeouts.WriteTotalTimeoutConstant = 200;
    timeouts.WriteTotalTimeoutMultiplier = 2;

    if (!SetCommTimeouts(handle_, &timeouts)) {
        close();
        return false;
    }

    PurgeComm(handle_, PURGE_RXCLEAR | PURGE_TXCLEAR);
    return true;
}

void SerialPort::close() {
    if (isOpen()) {
        CancelIo(handle_);
        PurgeComm(handle_, PURGE_RXCLEAR | PURGE_TXCLEAR);
        EscapeCommFunction(handle_, CLRDTR);
        EscapeCommFunction(handle_, CLRRTS);
        CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
    }
}

bool SerialPort::isOpen() const {
    return handle_ != INVALID_HANDLE_VALUE;
}

bool SerialPort::writeLine(const std::string& text) {
    if (!isOpen()) {
        return false;
    }

    const std::string payload = text + "\r\n";
    DWORD written = 0;
    return WriteFile(handle_, payload.data(), static_cast<DWORD>(payload.size()), &written, nullptr)
        && written == payload.size();
}

std::string SerialPort::readAvailable(DWORD waitMs) {
    if (!isOpen()) {
        return {};
    }

    // 中文注释：短时间轮询串口缓冲区，避免程序卡死在一次阻塞读取上。
    const ULONGLONG deadline = GetTickCount64() + waitMs;
    std::string result;
    std::vector<char> buffer(256);

    while (GetTickCount64() < deadline) {
        DWORD bytesRead = 0;
        if (ReadFile(handle_, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, nullptr) && bytesRead > 0) {
            result.append(buffer.data(), buffer.data() + bytesRead);
            continue;
        }

        Sleep(30);
    }

    return result;
}

const std::string& SerialPort::portName() const {
    return portName_;
}

std::string SerialPort::windowsDeviceName() const {
    if (portName_.rfind("\\\\.\\", 0) == 0) {
        return portName_;
    }
    return "\\\\.\\" + portName_;
}
