#include "SerialPort.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

namespace {

void printBlock(const std::string& title, const std::string& text) {
    std::cout << "----- " << title << " -----\n";
    if (text.empty()) {
        std::cout << "(empty)\n";
    } else {
        std::cout << text;
        if (text.back() != '\n') {
            std::cout << '\n';
        }
    }
}

bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

} // namespace

int main(int argc, char* argv[]) {
    const std::string portName = argc >= 2 ? argv[1] : "COM11";
    constexpr DWORD baudRate = 115200;

    std::cout << "Embedded AI PC Bridge\n";
    std::cout << "Port: " << portName << " @ " << baudRate << "\n";

    SerialPort serial(portName, baudRate);
    if (!serial.open()) {
        std::cerr << "ERROR: Cannot open serial port " << portName << ".\n";
        std::cerr << "Check whether the NUCLEO is connected, whether the COM port changed, or whether another program is using it.\n";
        return EXIT_FAILURE;
    }

    std::cout << "Serial port opened.\n";

    // 中文注释：打开串口后等待 STM32 复位/启动文本进入缓冲区。
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    const std::string bootText = serial.readAvailable(1000);
    printBlock("BOOT TEXT", bootText);

    std::cout << "Sending PING...\n";
    if (!serial.writeLine("PING")) {
        std::cerr << "ERROR: Failed to write PING.\n";
        return EXIT_FAILURE;
    }

    const std::string pongText = serial.readAvailable(1200);
    printBlock("PING REPLY", pongText);
    if (!contains(pongText, "PONG")) {
        std::cerr << "ERROR: Expected PONG but did not receive it.\n";
        return EXIT_FAILURE;
    }

    std::cout << "Sending LEDON...\n";
    serial.writeLine("LEDON");
    const std::string ledOnText = serial.readAvailable(800);
    printBlock("LEDON REPLY", ledOnText);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "Sending LEDOFF...\n";
    serial.writeLine("LEDOFF");
    const std::string ledOffText = serial.readAvailable(800);
    printBlock("LEDOFF REPLY", ledOffText);

    const bool ledOk = contains(ledOnText, "OK LED ON") && contains(ledOffText, "OK LED OFF");
    if (!ledOk) {
        std::cerr << "WARNING: PING/PONG works, but LED command replies were incomplete.\n";
        return EXIT_SUCCESS;
    }

    std::cout << "Bridge test passed: PC C++ <-> NUCLEO serial link is working.\n";
    return EXIT_SUCCESS;
}
