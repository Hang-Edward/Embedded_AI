#pragma once

#include "HttpClient.h"
#include "QwenResponseParser.h"
#include "QwenVisionConfig.h"

#include <string>

struct SpeechRecognitionResult {
    bool success = false;
    std::string transcript;
    std::string message;
    std::string sourceAudioPath;
};

class QwenAsrService {
public:
    QwenAsrService(QwenVisionConfig config, HttpClient& httpClient);

    SpeechRecognitionResult transcribeWav(const std::string& audioPath);
    std::string buildRequestBody(const std::string& audioDataUrl) const;

private:
    static std::string buildWavDataUrl(const std::string& audioPath);

    QwenVisionConfig config_;
    HttpClient& httpClient_;
    QwenResponseParser parser_;
};
