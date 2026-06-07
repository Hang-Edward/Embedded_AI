#include "AppConfig.h"

#include <QDir>
#include <QSettings>
#include <QStringList>
#include <QStandardPaths>

QStringList AppConfig::candidateHosts() const {
    QStringList hosts;
    if (!lastSuccessfulIp.trimmed().isEmpty()) {
        hosts << lastSuccessfulIp.trimmed();
    }
    hosts << fallbackIp.trimmed();
    const QString manual = normalizedManualHost();
    if (!manual.isEmpty() && !hosts.contains(manual)) {
        hosts << manual;
    }
    hosts.removeDuplicates();
    return hosts;
}

QString AppConfig::normalizedManualHost() const {
    QString value = manualSshCommand.trimmed();
    if (value.startsWith("ssh ")) {
        value = value.mid(4).trimmed();
    }
    if (value.contains("@")) {
        value = value.section("@", 1, 1).trimmed();
    }
    return value;
}

void AppConfig::load() {
    QSettings settings(settingsFilePath(), QSettings::IniFormat);
    username = settings.value("ssh/username", username).toString();
    fallbackIp = settings.value("ssh/fallbackIp", fallbackIp).toString();
    lastSuccessfulIp = settings.value("ssh/lastSuccessfulIp", lastSuccessfulIp).toString();
    manualSshCommand = settings.value("ssh/manualCommand", manualSshCommand).toString();
    projectPath = settings.value("paths/project", projectPath).toString();
    logPath = settings.value("paths/log", logPath).toString();
    framePath = settings.value("paths/frame", framePath).toString();
    authMode = settings.value("ssh/authMode", authMode).toString();
}

void AppConfig::save() const {
    QSettings settings(settingsFilePath(), QSettings::IniFormat);
    settings.setValue("ssh/username", username);
    settings.setValue("ssh/fallbackIp", fallbackIp);
    settings.setValue("ssh/lastSuccessfulIp", lastSuccessfulIp);
    settings.setValue("ssh/manualCommand", manualSshCommand);
    settings.setValue("paths/project", projectPath);
    settings.setValue("paths/log", logPath);
    settings.setValue("paths/frame", framePath);
    settings.setValue("ssh/authMode", authMode);
}

QString AppConfig::settingsFilePath() const {
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(base);
    return QDir(base).filePath("embedded-ai-control-center.ini");
}
