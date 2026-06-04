#include "HardwareBridge.h"
#include "SerialPort.h"

#include <cstdlib>
#include <string>

namespace {

void writeText(HANDLE stream, const std::string& text) {
    DWORD written = 0;
    WriteFile(stream, text.data(), static_cast<DWORD>(text.size()), &written, nullptr);
}

void logInfo(const std::string& text) {
    writeText(GetStdHandle(STD_OUTPUT_HANDLE), text);
}

void logError(const std::string& text) {
    writeText(GetStdHandle(STD_ERROR_HANDLE), text);
}

void printBlock(const std::string& title, const std::string& text) {
    logInfo("----- " + title + " -----\n");
    if (text.empty()) {
        logInfo("(empty)\n");
    } else {
        logInfo(text);
        if (text.back() != '\n') {
            logInfo("\n");
        }
    }
}

} // namespace

int main(int argc, char* argv[]) {
    const std::string portName = argc >= 2 ? argv[1] : "COM11";
    constexpr DWORD baudRate = 115200;

    logInfo("Embedded AI PC Bridge\n");
    logInfo("Port: " + portName + " @ " + std::to_string(baudRate) + "\n");

    SerialPort serial(portName, baudRate);
    if (!serial.open()) {
        logError("ERROR: Cannot open serial port " + portName + ".\n");
        logError("Check whether the NUCLEO is connected, whether the COM port changed, or whether another program is using it.\n");
        return EXIT_FAILURE;
    }

    logInfo("Serial port opened.\n");
    HardwareBridge bridge(serial);

    // 中文注释：打开串口后等待 STM32 复位/启动文本进入缓冲区。
    Sleep(500);
    const std::string bootText = serial.readAvailable(1000);
    printBlock("BOOT TEXT", bootText);

    logInfo("Sending PING...\n");
    if (!bridge.ping()) {
        logError("ERROR: Expected PONG but did not receive it.\n");
        return EXIT_FAILURE;
    }
    logInfo("PING OK.\n");

    logInfo("Testing LED...\n");
    const bool ledOnOk = bridge.setLed(true);

    Sleep(500);

    const bool ledOffOk = bridge.setLed(false);
    if (!ledOnOk || !ledOffOk) {
        logError("ERROR: LED command replies were incomplete.\n");
        return EXIT_FAILURE;
    }

    logInfo("Testing BUZZER/VIB/OLED protocol states...\n");
    const bool buzzerOk = bridge.setBuzzer(true) && bridge.setBuzzer(false);
    const bool vibrationOk = bridge.setVibration(true) && bridge.setVibration(false);
    const bool oledOk = bridge.showOledText("AI Bridge Ready");

    if (!buzzerOk || !vibrationOk || !oledOk) {
        logError("ERROR: Extended protocol command replies were incomplete.\n");
        return EXIT_FAILURE;
    }

    const std::string status = bridge.readStatus();
    printBlock("STATUS", status);

    serial.close();
    logInfo("Bridge test passed: unified PC C++ <-> NUCLEO protocol is working.\n");
    ExitProcess(EXIT_SUCCESS);
}
