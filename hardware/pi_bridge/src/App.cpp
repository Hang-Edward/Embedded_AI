#include "App.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <sstream>

namespace {

std::string trimAsciiWhitespace(std::string value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
        value.pop_back();
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
        value.erase(value.begin());
    }
    return value;
}

bool hasUsefulSpeechText(const std::string& transcript) {
    const std::string trimmed = trimAsciiWhitespace(transcript);
    return trimmed.size() >= 2U;
}

} // namespace

App::App(Console& console,
    HardwareBridge& hardware,
    PrototypeDeviceSet& devices,
    CameraService& camera,
    AiVisionService& aiVision,
    AudioRecorder& audioRecorder,
    QwenAsrService& asrService,
    AuditLogStore& auditLog)
    : console_(console),
      hardware_(hardware),
      devices_(devices),
      camera_(camera),
      aiVision_(aiVision),
      audioRecorder_(audioRecorder),
      asrService_(asrService),
      auditLog_(auditLog),
      hud_("."),
      rotary_() {
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
        case 9:
            analyzeVoiceCommand();
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

int App::runButtonMode() {
    console_.info("\nRotary voice assistant mode\n");
    console_.info("Press the rotary encoder button to start voice input.\n");
    console_.info("Rotate left/right to browse AI reply history on the LCD.\n");
    console_.info("The NUCLEO blue button is kept as a backup trigger.\n");
    console_.info("If speech is empty or ASR fails, the system will describe the current scene by default.\n");
    console_.info("Use Ctrl+C to stop this program.\n");
    hud_.showStatus(HudStatus::Ready, "系统就绪，可以按旋钮开始语音输入。");
    hardware_.readEvents(200);

    while (true) {
        const RotaryEvent rotaryEvent = rotary_.poll();
        if (rotaryEvent == RotaryEvent::CounterClockwise) {
            console_.info("Rotary: older reply page.\n");
            hud_.showOlderReplyPage();
            continue;
        }
        if (rotaryEvent == RotaryEvent::Clockwise) {
            console_.info("Rotary: newer reply page.\n");
            hud_.showNewerReplyPage();
            continue;
        }

        if (rotaryEvent == RotaryEvent::Pressed || waitForButtonEvent()) {
            console_.info("\nTrigger received. Starting voice command flow.\n");
            hud_.showStatus(HudStatus::Busy, "已触发，准备录音。");
            const uint32_t recordId = analyzeVoiceCommand();
            if (recordId == 0U) {
                hud_.showError("AI 流程失败，请查看日志。");
            }
            console_.info("\nReady. Press the rotary encoder button again for the next command.\n");
        }
    }
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
    console_.info("9. Voice command with current camera frame\n");
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
        hud_.showError("摄像头故障：" + result.message);
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
    hud_.showStatus(HudStatus::Busy, "AI 响应中，请稍等。");
    const CaptureResult capture = captureCameraFrame();
    if (!capture.success) {
        return 0;
    }

    const VisionAnalysisResult analysis = aiVision_.analyzeImage(capture.filePath, intent);
    if (!analysis.success) {
        auditLog_.appendHardwareAction("AI_ANALYZE", "FAILED: " + analysis.message);
        devices_.displayMessage("AI FAIL");
        hud_.showError("AI 分析失败：" + analysis.message);
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
    hud_.showReply(task.aiSummary);
    return entry.id;
}

uint32_t App::analyzeVoiceCommand() {
    std::string transcript;
    bool useSpeechPrompt = false;

    console_.info("Recording voice command for 5 seconds. Please speak now...\n");
    hud_.showRecordingCountdown(5);
    const AudioRecordResult audio = audioRecorder_.recordWav("captures/voice-command.wav", 5);
    if (!audio.success) {
        auditLog_.appendHardwareAction("VOICE_RECORD", "FAILED: " + audio.message);
        hud_.showError("录音失败：" + audio.message);
        console_.error("Voice recording failed: " + audio.message + "\n");
        console_.info("Fallback: describing current scene without voice command.\n");
    } else {
        auditLog_.appendHardwareAction("VOICE_RECORD", audio.filePath);
        console_.info("Audio recorded: " + audio.filePath + "\n");
        console_.info("Recognizing speech with Qwen ASR...\n");
        hud_.showStatus(HudStatus::Busy, "正在识别语音，请稍等。");

        const SpeechRecognitionResult speech = asrService_.transcribeWav(audio.filePath);
        if (!speech.success) {
            auditLog_.appendHardwareAction("VOICE_ASR", "FAILED: " + speech.message);
            console_.error("Speech recognition failed: " + speech.message + "\n");
            console_.info("Fallback: describing current scene without voice command.\n");
        } else {
            transcript = trimAsciiWhitespace(speech.transcript);
            auditLog_.appendHardwareAction("VOICE_ASR", transcript.empty() ? "(empty transcript)" : transcript);
            console_.info("Recognized command: " + (transcript.empty() ? "(empty)" : transcript) + "\n");
            useSpeechPrompt = hasUsefulSpeechText(transcript);
            if (!useSpeechPrompt) {
                console_.info("Fallback: voice command is empty or too short, describing current scene.\n");
            }
        }
    }

    const TaskType intent = useSpeechPrompt ? inferTaskTypeFromSpeech(transcript) : TaskType::SceneDescription;
    hud_.showStatus(HudStatus::Busy, "AI 响应中，请稍等。");
    const CaptureResult capture = captureCameraFrame();
    if (!capture.success) {
        hud_.showError("摄像头拍照失败：" + capture.message);
        return 0;
    }

    const VisionAnalysisResult analysis = useSpeechPrompt
        ? aiVision_.analyzeImageWithPrompt(capture.filePath, intent, buildVoiceVisionPrompt(transcript, intent))
        : aiVision_.analyzeImage(capture.filePath, TaskType::SceneDescription);

    if (!analysis.success) {
        auditLog_.appendHardwareAction("VOICE_VISION_ANALYZE", "FAILED: " + analysis.message);
        devices_.displayMessage("AI FAIL");
        hud_.showError("AI 响应失败：" + analysis.message);
        console_.error("Voice vision analysis failed: " + analysis.message + "\n");
        return 0;
    }

    const SceneTask task = createTaskFromVision(analysis);
    applyTaskFeedback(task);

    const AuditLogEntry entry = auditLog_.appendSceneTask(task);
    console_.info("Voice vision analysis recorded. ID=" + std::to_string(entry.id) + "\n");
    console_.info("Image=" + analysis.sourceImagePath + "\n");
    console_.info("Type=" + taskTypeToText(task.type) + ", Risk=" + taskRiskToText(task.risk) + "\n");
    console_.info(task.aiSummary + "\n");
    hud_.showReply(task.aiSummary);
    return entry.id;
}

bool App::waitForButtonEvent() {
    const std::string events = hardware_.readEvents(50);
    return events.find("EVENT BUTTON PRESSED") != std::string::npos;
}

TaskType App::inferTaskTypeFromSpeech(const std::string& transcript) const {
    if (transcript.find("解题") != std::string::npos
        || transcript.find("题目") != std::string::npos
        || transcript.find("答案") != std::string::npos
        || transcript.find("solve") != std::string::npos
        || transcript.find("problem") != std::string::npos) {
        return TaskType::ProblemSolving;
    }

    if (transcript.find("风险") != std::string::npos
        || transcript.find("危险") != std::string::npos
        || transcript.find("安全") != std::string::npos
        || transcript.find("电路") != std::string::npos
        || transcript.find("risk") != std::string::npos
        || transcript.find("safe") != std::string::npos) {
        return TaskType::RiskAlert;
    }

    return TaskType::SceneDescription;
}

std::string App::buildVoiceVisionPrompt(const std::string& transcript, TaskType intent) const {
    std::string taskHint;
    switch (intent) {
    case TaskType::SceneDescription:
        taskHint = "用户大概率想让你描述当前画面。";
        break;
    case TaskType::ProblemSolving:
        taskHint = "用户大概率想让你读取并讲解画面中的题目。请给出步骤化提示，不要只给最终答案。";
        break;
    case TaskType::RiskAlert:
        taskHint = "用户大概率想让你检查画面中的硬件或环境风险。请重点关注电源、地线、短路、松动线材和安全隐患。";
        break;
    }

    return "你是一个运行在嵌入式 AI 原型上的视觉助手。用户刚才通过麦克风说：\""
        + transcript
        + "\"。"
        + taskHint
        + "请结合图片内容，用中文直接回答用户。回答要适合课堂演示，清晰、简洁、可操作。";
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
