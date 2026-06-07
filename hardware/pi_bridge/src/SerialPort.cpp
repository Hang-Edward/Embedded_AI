#include "SerialPort.h"

#include <chrono>
#include <thread>
#include <utility>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace {

#if !defined(_WIN32)
speed_t toPosixBaud(std::uint32_t baudRate) {
    switch (baudRate) {
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    default:
        return B115200;
    }
}
#endif

} // namespace

SerialPort::SerialPort(std::string portName, std::uint32_t baudRate)
    : portName_(std::move(portName)), baudRate_(baudRate)
#if defined(_WIN32)
    , handle_(INVALID_HANDLE_VALUE)
#else
    , fd_(-1)
#endif
{
}

SerialPort::~SerialPort() {
    close();
}

bool SerialPort::open() {
    if (isOpen()) {
        return true;
    }

#if defined(_WIN32)
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
#else
    fd_ = ::open(portName_.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd_ < 0) {
        return false;
    }

    termios tty {};
    if (tcgetattr(fd_, &tty) != 0) {
        close();
        return false;
    }

    cfmakeraw(&tty);
    const speed_t speed = toPosixBaud(baudRate_);
    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    tty.c_cflag |= static_cast<tcflag_t>(CLOCAL | CREAD);
    tty.c_cflag &= static_cast<tcflag_t>(~PARENB);
    tty.c_cflag &= static_cast<tcflag_t>(~CSTOPB);
    tty.c_cflag &= static_cast<tcflag_t>(~CSIZE);
    tty.c_cflag |= CS8;
    tty.c_cflag &= static_cast<tcflag_t>(~CRTSCTS);
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
        close();
        return false;
    }

    tcflush(fd_, TCIOFLUSH);
    return true;
#endif
}

void SerialPort::close() {
    if (isOpen()) {
#if defined(_WIN32)
        CancelIo(handle_);
        PurgeComm(handle_, PURGE_RXCLEAR | PURGE_TXCLEAR);
        EscapeCommFunction(handle_, CLRDTR);
        EscapeCommFunction(handle_, CLRRTS);
        CloseHandle(handle_);
        handle_ = INVALID_HANDLE_VALUE;
#else
        ::close(fd_);
        fd_ = -1;
#endif
    }
}

bool SerialPort::isOpen() const {
#if defined(_WIN32)
    return handle_ != INVALID_HANDLE_VALUE;
#else
    return fd_ >= 0;
#endif
}

bool SerialPort::writeLine(const std::string& text) {
    if (!isOpen()) {
        return false;
    }

    const std::string payload = text + "\r\n";
#if defined(_WIN32)
    DWORD written = 0;
    return WriteFile(handle_, payload.data(), static_cast<DWORD>(payload.size()), &written, nullptr)
        && written == payload.size();
#else
    const ssize_t written = ::write(fd_, payload.data(), payload.size());
    return written == static_cast<ssize_t>(payload.size());
#endif
}

std::string SerialPort::readAvailable(std::uint32_t waitMs) {
    if (!isOpen()) {
        return {};
    }

    // 中文注释：短时间轮询串口缓冲区，避免程序卡死在一次阻塞读取上。
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(waitMs);
    std::string result;
    std::vector<char> buffer(256);

    while (std::chrono::steady_clock::now() < deadline) {
#if defined(_WIN32)
        DWORD bytesRead = 0;
        if (ReadFile(handle_, buffer.data(), static_cast<DWORD>(buffer.size()), &bytesRead, nullptr) && bytesRead > 0) {
            result.append(buffer.data(), buffer.data() + bytesRead);
            continue;
        }
#else
        const ssize_t bytesRead = ::read(fd_, buffer.data(), buffer.size());
        if (bytesRead > 0) {
            result.append(buffer.data(), buffer.data() + bytesRead);
            continue;
        }
        if (bytesRead < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            break;
        }
#endif

        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }

    return result;
}

const std::string& SerialPort::portName() const {
    return portName_;
}

#if defined(_WIN32)
std::string SerialPort::windowsDeviceName() const {
    if (portName_.rfind("\\\\.\\", 0) == 0) {
        return portName_;
    }
    return "\\\\.\\" + portName_;
}
#endif
