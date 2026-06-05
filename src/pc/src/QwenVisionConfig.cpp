#include "QwenVisionConfig.h"

#include "KeyValueConfigFile.h"

#include <cstdlib>
#include <fstream>

bool QwenVisionConfig::hasApiKey() const {
    return !apiKey.empty();
}

std::string QwenVisionConfig::endpoint() const {
    return baseUrl + "/chat/completions";
}

QwenVisionConfig QwenVisionConfigLoader::load(const std::string& configFilePath) const {
    QwenVisionConfig config;
    config.configFilePath = configFilePath;

    KeyValueConfigFile file;
    if (file.load(configFilePath)) {
        config.baseUrl = trimTrailingSlash(file.getString("base_url", config.baseUrl));
        config.model = file.getString("model", config.model);
        config.apiKeyFilePath = file.getString("api_key_file", config.apiKeyFilePath);
        config.apiKeyEnvName = file.getString("api_key_env", config.apiKeyEnvName);
        config.enabled = file.getString("enabled", "false") == "true";
    }

    config.apiKey = readSecretFile(config.apiKeyFilePath);
    if (config.apiKey.empty()) {
        config.apiKey = readEnv(config.apiKeyEnvName);
    }
    return config;
}

std::string QwenVisionConfigLoader::readEnv(const std::string& name) {
    if (name.empty()) {
        return {};
    }

    const char* value = std::getenv(name.c_str());
    if (value == nullptr) {
        return {};
    }

    return trimWhitespace(value);
}

std::string QwenVisionConfigLoader::readSecretFile(const std::string& filePath) {
    if (filePath.empty()) {
        return {};
    }

    std::ifstream file(filePath);
    if (!file) {
        return {};
    }

    std::string value;
    std::getline(file, value);
    return trimWhitespace(value);
}

std::string QwenVisionConfigLoader::trimTrailingSlash(std::string value) {
    while (!value.empty() && value.back() == '/') {
        value.pop_back();
    }
    return value;
}

std::string QwenVisionConfigLoader::trimWhitespace(std::string value) {
    while (!value.empty() && (value.back() == '\r' || value.back() == '\n' || value.back() == ' ' || value.back() == '\t')) {
        value.pop_back();
    }
    while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
        value.erase(value.begin());
    }
    return value;
}
