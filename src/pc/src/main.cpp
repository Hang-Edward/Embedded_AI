#include "App.h"
#include "AuditLogStore.h"
#include "Console.h"
#include "HardwareBridge.h"
#include "SerialPort.h"

#include <cstdlib>
#include <string>

namespace {

struct ProgramOptions {
    std::string portName = "COM11";
    bool demoMode = false;
};

ProgramOptions parseOptions(int argc, char* argv[]) {
    ProgramOptions options;
    for (int i = 1; i < argc; ++i) {
        const std::string value = argv[i];
        if (value == "--demo") {
            options.demoMode = true;
        } else {
            options.portName = value;
        }
    }
    return options;
}

} // namespace

int main(int argc, char* argv[]) {
    const ProgramOptions options = parseOptions(argc, argv);
    constexpr DWORD baudRate = 115200;

    Console console;
    console.info("Embedded AI Reality Bridge\n");
    console.info("Port: " + options.portName + " @ " + std::to_string(baudRate) + "\n");

    SerialPort serial(options.portName, baudRate);
    if (!serial.open()) {
        console.error("ERROR: Cannot open serial port " + options.portName + ".\n");
        console.error("Check whether the NUCLEO is connected, whether the COM port changed, or whether another program is using it.\n");
        return EXIT_FAILURE;
    }

    HardwareBridge bridge(serial);
    AuditLogStore auditLog("audit-log.dat");
    App app(console, bridge, auditLog);
    const int exitCode = options.demoMode ? app.runDemo() : app.runInteractive();

    serial.close();
    ExitProcess(static_cast<UINT>(exitCode));
}
