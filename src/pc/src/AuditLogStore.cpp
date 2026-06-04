#include "AuditLogStore.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>

struct AuditLogStore::Record {
    uint32_t magic = AuditLogStore::kMagic;
    uint32_t version = AuditLogStore::kVersion;
    uint32_t id = 0;
    uint32_t status = static_cast<uint32_t>(AuditStatus::Pending);
    char timestamp[32] {};
    char action[32] {};
    char detail[160] {};
    char risk[16] {};
};

namespace {

std::string trimFixedText(const char* text, size_t size) {
    const auto end = std::find(text, text + size, '\0');
    return std::string(text, end);
}

} // namespace

AuditLogStore::AuditLogStore(std::string filePath)
    : filePath_(std::move(filePath)) {
}

AuditLogEntry AuditLogStore::appendHardwareAction(const std::string& action, const std::string& detail) {
    AuditLogEntry entry;
    entry.id = nextId();
    entry.timestamp = currentTimestamp();
    entry.action = action;
    entry.detail = detail;
    entry.risk = "LOW";
    entry.status = AuditStatus::Confirmed;

    std::ofstream file(filePath_, std::ios::binary | std::ios::app);
    const Record record = toRecord(entry);
    file.write(reinterpret_cast<const char*>(&record), sizeof(record));
    return entry;
}

AuditLogEntry AuditLogStore::appendSceneTask(const SceneTask& task) {
    AuditLogEntry entry;
    entry.id = nextId();
    entry.timestamp = currentTimestamp();
    entry.action = "SCENE_TASK";
    entry.detail = task.title + " | " + task.aiSummary;
    entry.risk = taskRiskToText(task.risk);
    entry.status = AuditStatus::Pending;

    std::ofstream file(filePath_, std::ios::binary | std::ios::app);
    const Record record = toRecord(entry);
    file.write(reinterpret_cast<const char*>(&record), sizeof(record));
    return entry;
}

std::vector<AuditLogEntry> AuditLogStore::readAll() const {
    std::vector<AuditLogEntry> entries;
    std::ifstream file(filePath_, std::ios::binary);
    Record record;
    while (file.read(reinterpret_cast<char*>(&record), sizeof(record))) {
        if (record.magic == kMagic && record.version == kVersion) {
            entries.push_back(fromRecord(record));
        }
    }
    return entries;
}

bool AuditLogStore::readById(uint32_t id, AuditLogEntry& outEntry) const {
    const std::streamoff offset = recordOffset(id);
    if (offset < 0) {
        return false;
    }

    std::ifstream file(filePath_, std::ios::binary);
    if (!file) {
        return false;
    }

    Record record;
    file.seekg(offset, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(&record), sizeof(record))) {
        return false;
    }

    if (record.magic != kMagic || record.version != kVersion || record.id != id) {
        return false;
    }

    outEntry = fromRecord(record);
    return true;
}

bool AuditLogStore::updateStatus(uint32_t id, AuditStatus status) {
    const std::streamoff offset = recordOffset(id);
    if (offset < 0) {
        return false;
    }

    std::fstream file(filePath_, std::ios::binary | std::ios::in | std::ios::out);
    if (!file) {
        return false;
    }

    Record record;
    file.seekg(offset, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(&record), sizeof(record))) {
        return false;
    }

    if (record.magic != kMagic || record.version != kVersion || record.id != id) {
        return false;
    }

    // 中文注释：这里是课程要求中的随机文件更新，直接 seek 到指定记录覆盖状态字段。
    record.status = static_cast<uint32_t>(status);
    file.seekp(offset, std::ios::beg);
    file.write(reinterpret_cast<const char*>(&record), sizeof(record));
    return true;
}

uint32_t AuditLogStore::nextId() const {
    std::ifstream file(filePath_, std::ios::binary);
    if (!file) {
        return 1;
    }

    file.seekg(0, std::ios::end);
    const auto bytes = file.tellg();
    if (bytes <= 0) {
        return 1;
    }
    return static_cast<uint32_t>(bytes / static_cast<std::streamoff>(sizeof(Record))) + 1U;
}

std::string AuditLogStore::currentTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t rawTime = std::chrono::system_clock::to_time_t(now);
    std::tm localTime {};
#if defined(_WIN32)
    localtime_s(&localTime, &rawTime);
#else
    localtime_r(&rawTime, &localTime);
#endif
    std::ostringstream out;
    out << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

AuditLogStore::Record AuditLogStore::toRecord(const AuditLogEntry& entry) {
    Record record;
    record.id = entry.id;
    record.status = static_cast<uint32_t>(entry.status);
    copyFixed(record.timestamp, sizeof(record.timestamp), entry.timestamp);
    copyFixed(record.action, sizeof(record.action), entry.action);
    copyFixed(record.detail, sizeof(record.detail), entry.detail);
    copyFixed(record.risk, sizeof(record.risk), entry.risk);
    return record;
}

AuditLogEntry AuditLogStore::fromRecord(const Record& record) {
    AuditLogEntry entry;
    entry.id = record.id;
    entry.timestamp = trimFixedText(record.timestamp, sizeof(record.timestamp));
    entry.action = trimFixedText(record.action, sizeof(record.action));
    entry.detail = trimFixedText(record.detail, sizeof(record.detail));
    entry.risk = trimFixedText(record.risk, sizeof(record.risk));
    entry.status = static_cast<AuditStatus>(record.status);
    return entry;
}

void AuditLogStore::copyFixed(char* destination, uint32_t size, const std::string& source) {
    if (size == 0U) {
        return;
    }

    const uint32_t copySize = static_cast<uint32_t>(std::min<size_t>(source.size(), size - 1U));
    std::fill(destination, destination + size, '\0');
    std::copy_n(source.data(), copySize, destination);
}

std::streamoff AuditLogStore::recordOffset(uint32_t id) {
    if (id == 0) {
        return -1;
    }
    return static_cast<std::streamoff>((id - 1U) * sizeof(Record));
}

std::string auditStatusToText(AuditStatus status) {
    switch (status) {
    case AuditStatus::Pending:
        return "PENDING";
    case AuditStatus::Confirmed:
        return "CONFIRMED";
    case AuditStatus::Ignored:
        return "IGNORED";
    }
    return "UNKNOWN";
}
