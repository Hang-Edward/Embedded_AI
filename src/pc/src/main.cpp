#include "App.h"
#include "AuditLogStore.h"
#include "Console.h"
#include "CurlHttpClient.h"
#include "HardwareBridge.h"
#include "MockAiVisionService.h"
#include "OpenCvCameraService.h"
#include "PrototypeDeviceSet.h"
#include "QwenAsrService.h"
#include "QwenVisionConfig.h"
#include "QwenVisionService.h"
#include "SerialPort.h"
#include "ShellAudioRecorder.h"

#include <cstdint>
#include <cstdlib>
#include <string>

namespace {

struct ProgramOptions {
#if defined(_WIN32)
    std::string portName = "COM11";
#else
    std::string portName = "/dev/ttyACM0";
#endif
    std::string qwenConfigPath = "config/qwen-vision.ini";
    int cameraIndex = -1;
    bool demoMode = false;
    bool useQwen = false;
    bool buttonMode = false;
};

ProgramOptions parseOptions(int argc, char* argv[]) {
    ProgramOptions options;
    for (int i = 1; i < argc; ++i) {
        const std::string value = argv[i];
        if (value == "--demo") {
            options.demoMode = true;
        } else if (value == "--button") {
            options.buttonMode = true;
        } else if (value == "--qwen") {
            options.useQwen = true;
        } else if (value == "--qwen-config" && i + 1 < argc) {
            options.qwenConfigPath = argv[++i];
        } else if (value == "--camera" && i + 1 < argc) {
            options.cameraIndex = std::stoi(argv[++i]);
        } else {
            options.portName = value;
        }
    }
    return options;
}

}

int main(int argc, char* argv[]) {
    const ProgramOptions options = parseOptions(argc, argv);
    constexpr std::uint32_t baudRate = 115200;

    Console console;
    console.info("Embedded AI Reality Bridge\n");
    console.info("Port: " + options.portName + " @ " + std::to_string(baudRate) + "\n");
    console.info(std::string("Vision mode: ") + (options.useQwen ? "Qwen API\n" : "Mock local\n"));
    console.info(std::string("Run mode: ") + (options.buttonMode ? "Button voice assistant\n" : "Interactive console\n"));

    SerialPort serial(options.portName, baudRate);
    if (!serial.open()) {
        console.error("ERROR: Cannot open serial port " + options.portName + ".\n");
        console.error("Check whether the NUCLEO is connected, whether the COM port changed, or whether another program is using it.\n");
        return EXIT_FAILURE;
    }

    HardwareBridge bridge(serial);
    PrototypeDeviceSet devices(bridge);
    OpenCvCameraService camera(options.cameraIndex);
    MockAiVisionService aiVision;
    CurlHttpClient httpClient;
    QwenVisionConfigLoader configLoader;
    QwenVisionConfig qwenConfig = configLoader.load(options.qwenConfigPath);
    ShellAudioRecorder audioRecorder(qwenConfig.audioDevice);
    QwenAsrService qwenAsr(qwenConfig, httpClient);
    QwenVisionService qwenVision(qwenConfig, httpClient);
    AiVisionService& selectedVision = options.useQwen ? static_cast<AiVisionService&>(qwenVision) : static_cast<AiVisionService&>(aiVision);
    AuditLogStore auditLog("audit-log.dat");
    App app(console, bridge, devices, camera, selectedVision, audioRecorder, qwenAsr, auditLog);
    int exitCode = EXIT_SUCCESS;
    if (options.buttonMode) {
        exitCode = app.runButtonMode();
    } else if (options.demoMode) {
        exitCode = app.runDemo();
    } else {
        exitCode = app.runInteractive();
    }

    serial.close();
    return exitCode;
}
