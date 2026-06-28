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
    QString deepSeekBaseUrl = "https://api.deepseek.com";
    QString deepSeekModel = "deepseek-v4-flash";
    QString deepSeekApiKeyFile = "config/deepseek.key";
    QString deepSeekApiKeyEnv = "EMBEDDED_AI_DEEPSEEK_KEY";
    QString qwenVisionBaseUrl = "https://dashscope.aliyuncs.com/compatible-mode/v1";
    QString qwenVisionModel = "qwen3-vl-8b-instruct";
    QString qwenVisionApiKeyFile = "config/qwen-vision.key";
    QString qwenVisionApiKeyEnv = "EMBEDDED_AI_QWEN_KEY";
    bool chatIncludeCurrentScene = true;

    QStringList candidateHosts() const;
    QString normalizedManualHost() const;
    void load();
    void save() const;

private:
    QString settingsFilePath() const;
};
