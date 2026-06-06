#pragma once

#include "AiVisionService.h"
#include "AudioRecorder.h"
#include "AuditLogStore.h"
#include "CameraService.h"
#include "Console.h"
#include "HardwareBridge.h"
#include "PrototypeDeviceSet.h"
#include "QwenAsrService.h"

class App {
public:
    App(Console& console,
        HardwareBridge& hardware,
        PrototypeDeviceSet& devices,
        CameraService& camera,
        AiVisionService& aiVision,
        AudioRecorder& audioRecorder,
        QwenAsrService& asrService,
        AuditLogStore& auditLog);

    int runInteractive();
    int runDemo();

private:
    void printMenu() const;
    void testHardware();
    void setLed(bool enabled);
    uint32_t simulateSceneTask(TaskType type);
    void simulateTaskMenu();
    CaptureResult captureCameraFrame();
    void analyzeCameraFrameMenu();
    uint32_t analyzeCurrentFrame(TaskType intent);
    uint32_t analyzeVoiceCommand();
    TaskType inferTaskTypeFromSpeech(const std::string& transcript) const;
    std::string buildVoiceVisionPrompt(const std::string& transcript, TaskType intent) const;
    SceneTask createMockTask(TaskType type) const;
    SceneTask createTaskFromVision(const VisionAnalysisResult& result) const;
    void applyTaskFeedback(const SceneTask& task);
    void showAuditLog() const;
    void updateAuditStatus();
    void printEntry(const AuditLogEntry& entry) const;

    Console& console_;
    HardwareBridge& hardware_;
    PrototypeDeviceSet& devices_;
    CameraService& camera_;
    AiVisionService& aiVision_;
    AudioRecorder& audioRecorder_;
    QwenAsrService& asrService_;
    AuditLogStore& auditLog_;
};
