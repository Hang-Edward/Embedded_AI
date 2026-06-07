#pragma once

#include <QString>
#include <QStringList>

class AppConfig {
public:
    QString username = "ch";
    QString fallbackIp = "172.20.10.6";
    QString lastSuccessfulIp;
    QString manualSshCommand;
    QString projectPath = "~/Embedded_AI";
    QString logPath = "~/Embedded_AI/logs/embedded-ai.log";
    QString framePath = "~/Embedded_AI/captures/latest-frame.jpg";
    QString authMode = "password";

    QStringList candidateHosts() const;
    QString normalizedManualHost() const;
    void load();
    void save() const;

private:
    QString settingsFilePath() const;
};
