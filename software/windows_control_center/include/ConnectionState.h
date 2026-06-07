#pragma once

#include <QString>
#include <QStringList>
#include <QList>

enum class HealthLevel {
    Unknown,
    Checking,
    Ok,
    Warning,
    Error
};

enum class AssistantStatus {
    Offline,
    Connecting,
    Ready,
    Listening,
    Thinking,
    Warning,
    Error
};

struct HealthItem {
    QString name;
    QString detail;
    HealthLevel level = HealthLevel::Unknown;
};

struct ConversationRecord {
    QString title;
    QString timestamp;
    QString userText;
    QString aiText;
    QString flowText;
    QString imagePath;
};

struct ConnectionState {
    bool piReachable = false;
    bool sshOnline = false;
    bool serviceActive = false;
    bool buttonReady = false;
    bool sameLanLikely = true;
    QString activeHost;
    QString warning;
    QString logText;
    QString localFramePath;
    QString latestSummary;
    QString assistantStatusText;
    AssistantStatus assistantStatus = AssistantStatus::Connecting;
    int voiceCountdownSeconds = 0;
    QStringList recentFramePaths;
    QStringList localIpv4Addresses;
    QList<HealthItem> hardwareItems;
    QList<ConversationRecord> recentRecords;
};

QString healthLevelText(HealthLevel level);
QString healthLevelClass(HealthLevel level);
