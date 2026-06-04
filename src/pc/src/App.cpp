#include "App.h"

#include <cstdlib>
#include <sstream>

App::App(Console& console, HardwareBridge& hardware, AuditLogStore& auditLog)
    : console_(console), hardware_(hardware), auditLog_(auditLog) {
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
            simulateSceneTask();
            break;
        case 5:
            showAuditLog();
            break;
        case 6:
            updateAuditStatus();
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
    const uint32_t sceneRecordId = simulateSceneTask();
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
    console_.info("4. Simulate AI scene result\n");
    console_.info("5. View audit log\n");
    console_.info("6. Update audit log status\n");
    console_.info("0. Exit\n");
}

void App::testHardware() {
    const bool ok = hardware_.ping();
    auditLog_.appendHardwareAction("PING", ok ? "PONG received" : "PONG missing");
    console_.info(ok ? "Hardware connection OK.\n" : "Hardware connection failed.\n");

    const std::string status = hardware_.readStatus();
    auditLog_.appendHardwareAction("STATUS", status);
    console_.info("Device status: " + status + "\n");
}

void App::setLed(bool enabled) {
    const bool ok = hardware_.setLed(enabled);
    auditLog_.appendHardwareAction(enabled ? "LED:ON" : "LED:OFF", ok ? "OK" : "FAILED");
    console_.info(ok ? "LED command OK.\n" : "LED command failed.\n");
}

uint32_t App::simulateSceneTask() {
    SceneTask task;
    task.title = "Mock scene understanding";
    task.prompt = "Describe current camera view";
    task.aiSummary = "No camera connected yet. Simulated answer: desk prototype is ready.";
    task.risk = TaskRisk::Low;

    hardware_.showOledText("Mock AI Result");
    hardware_.setBuzzer(false);
    hardware_.setVibration(false);

    const AuditLogEntry entry = auditLog_.appendSceneTask(task);
    console_.info("Scene task recorded. ID=" + std::to_string(entry.id) + "\n");
    return entry.id;
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
