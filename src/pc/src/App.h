#pragma once

#include "AuditLogStore.h"
#include "Console.h"
#include "HardwareBridge.h"

class App {
public:
    App(Console& console, HardwareBridge& hardware, AuditLogStore& auditLog);

    int runInteractive();
    int runDemo();

private:
    void printMenu() const;
    void testHardware();
    void setLed(bool enabled);
    uint32_t simulateSceneTask();
    void showAuditLog() const;
    void updateAuditStatus();
    void printEntry(const AuditLogEntry& entry) const;

    Console& console_;
    HardwareBridge& hardware_;
    AuditLogStore& auditLog_;
};
