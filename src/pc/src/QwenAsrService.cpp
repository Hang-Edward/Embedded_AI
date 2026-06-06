#include "QwenAsrService.h"

#include "Base64Encoder.h"

#include <cstdint>
#include <fstream>
#include <iterator>
#include <map>
#include <sstream>
#include <utility>
#include <vector>

namespace {

std::string jsonEscape(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size() + 16U);

    for (const char ch : value) {
        switch (ch) {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped.push_back(ch);
            break;
        }
    }

    return escaped;
}

} // namespace

QwenAsrService::QwenAsrService(QwenVisionConfig config, HttpClient& httpClient)
    : config_(std::move(config)), httpClient_(httpClient) {
}

SpeechRecognitionResult QwenAsrService::transcribeWav(const std::string& audioPath) {
    SpeechRecognitionResult result;
    result.sourceAudioPath = audioPath;

    if (!config_.enabled) {
        result.message = "Qwen ASR is disabled. Set enabled=true in config/qwen-vision.ini.";
        return result;
    }

    if (!config_.hasApiKey()) {
        result.message = "Missing API key. Put it in " + config_.apiKeyFilePath
            + " or set environment variable " + config_.apiKeyEnvName + ".";
        return result;
    }

    const std::string audioDataUrl = buildWavDataUrl(audioPath);
    if (audioDataUrl.empty()) {
        result.message = "Failed to read audio file for Qwen ASR request: " + audioPath;
        return result;
    }

    const std::map<std::string, std::string> headers {
        {"Authorization", "Bearer " + config_.apiKey},
    };
    const HttpResponse response = httpClient_.postJson(config_.endpoint(), headers, buildRequestBody(audioDataUrl));
    if (!response.ok()) {
        result.message = "Qwen ASR HTTP request failed. status=" + std::to_string(response.statusCode);
        const std::string apiError = parser_.extractErrorMessage(response.body);
        if (!apiError.empty()) {
            result.message += " api_error=" + apiError;
        }
        if (!response.errorMessage.empty()) {
            result.message += " transport_error=" + response.errorMessage;
        }
        return result;
    }

    result.transcript = parser_.extractAssistantText(response.body);
    if (result.transcript.empty()) {
        result.message = "Qwen ASR response did not contain recognized text.";
        return result;
    }

    result.success = true;
    result.message = "Speech recognized.";
    return result;
}

std::string QwenAsrService::buildRequestBody(const std::string& audioDataUrl) const {
    std::ostringstream body;
    body << "{";
    body << "\"model\":\"" << jsonEscape(config_.asrModel) << "\",";
    body << "\"messages\":[{";
    body << "\"role\":\"user\",";
    body << "\"content\":[{";
    body << "\"type\":\"input_audio\",";
    body << "\"input_audio\":{\"data\":\"" << jsonEscape(audioDataUrl) << "\"}";
    body << "}]";
    body << "}],";
    body << "\"stream\":false,";
    body << "\"asr_options\":{\"enable_itn\":false}";
    body << "}";
    return body.str();
}

std::string QwenAsrService::buildWavDataUrl(const std::string& audioPath) {
    std::ifstream file(audioPath, std::ios::binary);
    if (!file) {
        return {};
    }

    const std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (bytes.empty()) {
        return {};
    }

    const Base64Encoder encoder;
    return "data:audio/wav;base64," + encoder.encode(bytes);
}
