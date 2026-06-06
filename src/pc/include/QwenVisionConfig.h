#pragma once

#include <string>

struct QwenVisionConfig {
    std::string baseUrl = "https://dashscope.aliyuncs.com/compatible-mode/v1";
    std::string model = "qwen3-vl-8b-instruct";
    std::string asrModel = "qwen3-asr-flash";
    std::string audioDevice = "default";
    std::string apiKeyFilePath = "config/qwen-vision.key";
    std::string apiKeyEnvName = "EMBEDDED_AI_QWEN_KEY";
    std::string apiKey;
    std::string configFilePath = "config/qwen-vision.ini";
    bool enabled = false;

    bool hasApiKey() const;
    std::string endpoint() const;
};

class QwenVisionConfigLoader {
public:
    QwenVisionConfig load(const std::string& configFilePath) const;

private:
    static std::string readEnv(const std::string& name);
    static std::string readSecretFile(const std::string& filePath);
    static std::string trimTrailingSlash(std::string value);
    static std::string trimWhitespace(std::string value);
};
