#pragma once

#include <QString>
#include <QStringList>

enum class HealthLevel {
    Unknown,
    Checking,
    Ok,
    Warning,
    Error
};

struct HealthItem {
    QString name;
    QString detail;
    HealthLevel level = HealthLevel::Unknown;
};

struct ConnectionState {
    bool piReachable = false;
    bool sshOnline = false;
    bool sameLanLikely = true;
    QString activeHost;
    QString warning;
    QString logText;
    QString localFramePath;
    QString latestSummary;
    QStringList recentFramePaths;
    QStringList localIpv4Addresses;
    QList<HealthItem> hardwareItems;
};

QString healthLevelText(HealthLevel level);
QString healthLevelClass(HealthLevel level);
