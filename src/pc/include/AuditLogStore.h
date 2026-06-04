#pragma once

#include "SceneTask.h"

#include <cstdint>
#include <string>
#include <vector>

enum class AuditStatus : uint32_t {
    Pending = 0,
    Confirmed = 1,
    Ignored = 2
};

struct AuditLogEntry {
    uint32_t id = 0;
    std::string timestamp;
    std::string action;
    std::string detail;
    std::string risk;
    AuditStatus status = AuditStatus::Pending;
};

class AuditLogStore {
public:
    explicit AuditLogStore(std::string filePath);

    AuditLogEntry appendHardwareAction(const std::string& action, const std::string& detail);
    AuditLogEntry appendSceneTask(const SceneTask& task);
    std::vector<AuditLogEntry> readAll() const;
    bool readById(uint32_t id, AuditLogEntry& outEntry) const;
    bool updateStatus(uint32_t id, AuditStatus status);
    uint32_t nextId() const;

private:
    struct Record;

    static constexpr uint32_t kMagic = 0x41495242U;
    static constexpr uint32_t kVersion = 1U;

    static std::string currentTimestamp();
    static Record toRecord(const AuditLogEntry& entry);
    static AuditLogEntry fromRecord(const Record& record);
    static void copyFixed(char* destination, uint32_t size, const std::string& source);
    static std::streamoff recordOffset(uint32_t id);

    std::string filePath_;
};

std::string auditStatusToText(AuditStatus status);
