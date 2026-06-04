#include "App.h"

#include <cstdlib>
#include <sstream>

App::App(Console& console,
    HardwareBridge& hardware,
    PrototypeDeviceSet& devices,
    CameraService& camera,
    AiVisionService& aiVision,
    AuditLogStore& auditLog)
    : console_(console), hardware_(hardware), devices_(devices), camera_(camera), aiVision_(aiVision), auditLog_(auditLog) {
}

int App::runInteractive() {
    console_.info("\nEmbedded AI Reality Bridge - PC Console\n");
    while (true) {
        printMenu();
        const int choice = console_.askInt("Select: ");
        switch (choice) {
        case 1:
            testHardware();
            break;
        case 2:
            setLed(true);
            break;
        case 3:
            setLed(false);
            break;
        case 4:
            simulateTaskMenu();
            break;
        case 5:
            showAuditLog();
            break;
        case 6:
            updateAuditStatus();
            break;
        case 7:
            captureCameraFrame();
            break;
        case 8:
            analyzeCameraFrameMenu();
            break;
        case 0:
            return EXIT_SUCCESS;
        default:
            console_.error("Unknown menu item.\n");
            break;
        }
    }
}

int App::runDemo() {
    console_.info("Running non-interactive demo...\n");
    testHardware();
    setLed(true);
    setLed(false);
    simulateSceneTask(TaskType::SceneDescription);
    analyzeCurrentFrame(TaskType::SceneDescription);
    analyzeCurrentFrame(TaskType::ProblemSolving);
    simulateSceneTask(TaskType::ProblemSolving);
    const uint32_t sceneRecordId = simulateSceneTask(TaskType::RiskAlert);
    showAuditLog();
    const bool updated = auditLog_.updateStatus(sceneRecordId, AuditStatus::Confirmed);
    console_.info(updated ? "Random-access update OK for scene record.\n" : "Random-access update failed for scene record.\n");
    AuditLogEntry updatedEntry;
    if (auditLog_.readById(sceneRecordId, updatedEntry)) {
        printEntry(updatedEntry);
    }
    console_.info("Demo finished.\n");
    return EXIT_SUCCESS;
}

void App::printMenu() const {
    console_.info("\n");
    console_.info("1. Test hardware connection\n");
    console_.info("2. Turn LED on\n");
    console_.info("3. Turn LED off\n");
    console_.info("4. Simulate AI task\n");
    console_.info("5. View audit log\n");
    console_.info("6. Update audit log status\n");
    console_.info("7. Capture camera frame\n");
    console_.info("8. Capture and analyze current frame\n");
    console_.info("0. Exit\n");
}

void App::testHardware() {
    const bool ok = hardware_.ping();
    auditLog_.appendHardwareAction("PING", ok ? "PONG received" : "PONG missing");
    console_.info(ok ? "Hardware connection OK.\n" : "Hardware connection failed.\n");

    const std::string status = hardware_.readStatus();
    auditLog_.appendHardwareAction("STATUS", status);
    console_.info("Device status: " + status + "\n");

    const bool deviceSelfTest = devices_.runSelfTest();
    auditLog_.appendHardwareAction("DEVICE_SELF_TEST", deviceSelfTest ? "OK" : "FAILED");
    console_.info(deviceSelfTest ? "Output device self-test OK.\n" : "Output device self-test failed.\n");
}

void App::setLed(bool enabled) {
    const bool ok = devices_.setLed(enabled);
    auditLog_.appendHardwareAction(enabled ? "LED:ON" : "LED:OFF", ok ? "OK" : "FAILED");
    console_.info(ok ? "LED command OK.\n" : "LED command failed.\n");
}

uint32_t App::simulateSceneTask(TaskType type) {
    const SceneTask task = createMockTask(type);
    applyTaskFeedback(task);

    const AuditLogEntry entry = auditLog_.appendSceneTask(task);
    console_.info("AI task recorded. ID=" + std::to_string(entry.id) + "\n");
    console_.info("Type=" + taskTypeToText(task.type) + ", Risk=" + taskRiskToText(task.risk) + "\n");
    console_.info(task.aiSummary + "\n");
    return entry.id;
}

void App::simulateTaskMenu() {
    console_.info("\n");
    console_.info("1. Describe current scene\n");
    console_.info("2. Give problem-solving hint\n");
    console_.info("3. Risk alert\n");
    const int choice = console_.askInt("Task type: ");

    switch (choice) {
    case 1:
        simulateSceneTask(TaskType::SceneDescription);
        break;
    case 2:
        simulateSceneTask(TaskType::ProblemSolving);
        break;
    case 3:
        simulateSceneTask(TaskType::RiskAlert);
        break;
    default:
        console_.error("Unknown task type.\n");
        break;
    }
}

CaptureResult App::captureCameraFrame() {
    const CaptureResult result = camera_.captureFrame("captures/latest-frame.jpg");
    if (result.success) {
        std::ostringstream detail;
        detail << result.filePath << " camera=" << result.cameraIndex
               << " size=" << result.width << "x" << result.height;
        auditLog_.appendHardwareAction("CAMERA_CAPTURE", detail.str());
        devices_.displayMessage("FRAME SAVED");
        console_.info("Camera frame captured: " + detail.str() + "\n");
    } else {
        auditLog_.appendHardwareAction("CAMERA_CAPTURE", "FAILED: " + result.message);
        devices_.displayMessage("CAMERA FAIL");
        console_.error("Camera capture failed: " + result.message + "\n");
    }
    return result;
}

void App::analyzeCameraFrameMenu() {
    console_.info("\n");
    console_.info("1. Describe current scene\n");
    console_.info("2. Read and solve problem\n");
    console_.info("3. Check hardware safety risk\n");
    const int choice = console_.askInt("Vision intent: ");

    switch (choice) {
    case 1:
        analyzeCurrentFrame(TaskType::SceneDescription);
        break;
    case 2:
        analyzeCurrentFrame(TaskType::ProblemSolving);
        break;
    case 3:
        analyzeCurrentFrame(TaskType::RiskAlert);
        break;
    default:
        console_.error("Unknown vision intent.\n");
        break;
    }
}

uint32_t App::analyzeCurrentFrame(TaskType intent) {
    const CaptureResult capture = captureCameraFrame();
    if (!capture.success) {
        return 0;
    }

    const VisionAnalysisResult analysis = aiVision_.analyzeImage(capture.filePath, intent);
    if (!analysis.success) {
        auditLog_.appendHardwareAction("AI_ANALYZE", "FAILED: " + analysis.message);
        devices_.displayMessage("AI FAIL");
        console_.error("AI analysis failed: " + analysis.message + "\n");
        return 0;
    }

    const SceneTask task = createTaskFromVision(analysis);
    applyTaskFeedback(task);

    const AuditLogEntry entry = auditLog_.appendSceneTask(task);
    console_.info("Vision analysis recorded. ID=" + std::to_string(entry.id) + "\n");
    console_.info("Image=" + analysis.sourceImagePath + "\n");
    console_.info("Type=" + taskTypeToText(task.type) + ", Risk=" + taskRiskToText(task.risk) + "\n");
    console_.info(task.aiSummary + "\n");
    return entry.id;
}

SceneTask App::createMockTask(TaskType type) const {
    SceneTask task;
    task.type = type;

    switch (type) {
    case TaskType::SceneDescription:
        task.title = "Describe desk prototype";
        task.prompt = "Describe current camera view";
        task.aiSummary = "Simulated answer: the desk prototype includes a NUCLEO board, USB cable, and PC console.";
        task.risk = TaskRisk::Low;
        break;
    case TaskType::ProblemSolving:
        task.title = "Math problem hint";
        task.prompt = "Explain the next step without giving only the final answer";
        task.aiSummary = "Simulated answer: identify known variables, write the formula, then substitute values step by step.";
        task.risk = TaskRisk::Medium;
        break;
    case TaskType::RiskAlert:
        task.title = "Unsafe scene warning";
        task.prompt = "Check whether the current scene has hardware risk";
        task.aiSummary = "Simulated answer: possible wiring risk detected. Confirm power and ground before continuing.";
        task.risk = TaskRisk::High;
        break;
    }

    return task;
}

SceneTask App::createTaskFromVision(const VisionAnalysisResult& result) const {
    SceneTask task;
    task.type = result.taskType;
    task.title = result.title;
    task.prompt = result.prompt;
    task.aiSummary = result.summary + " Source image: " + result.sourceImagePath;
    task.risk = result.risk;
    return task;
}

void App::applyTaskFeedback(const SceneTask& task) {
    devices_.applyFeedback(task);
}

void App::showAuditLog() const {
    const auto entries = auditLog_.readAll();
    if (entries.empty()) {
        console_.info("Audit log is empty.\n");
        return;
    }

    for (const auto& entry : entries) {
        printEntry(entry);
    }
}

void App::updateAuditStatus() {
    const int id = console_.askInt("Record ID: ");
    console_.info("1. CONFIRMED\n");
    console_.info("2. IGNORED\n");
    const int statusChoice = console_.askInt("New status: ");

    AuditStatus status = AuditStatus::Pending;
    if (statusChoice == 1) {
        status = AuditStatus::Confirmed;
    } else if (statusChoice == 2) {
        status = AuditStatus::Ignored;
    } else {
        console_.error("Unsupported status.\n");
        return;
    }

    const bool ok = auditLog_.updateStatus(static_cast<uint32_t>(id), status);
    console_.info(ok ? "Record updated by random-access write.\n" : "Record update failed.\n");
}

void App::printEntry(const AuditLogEntry& entry) const {
    std::ostringstream out;
    out << "#" << entry.id
        << " [" << entry.timestamp << "]"
        << " action=" << entry.action
        << " risk=" << entry.risk
        << " status=" << auditStatusToText(entry.status)
        << "\n  " << entry.detail << "\n";
    console_.info(out.str());
}
