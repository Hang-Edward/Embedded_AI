#pragma once

#include "AppConfig.h"

#include <QString>

struct VisionRecognitionResult {
    bool success = false;
    QString summary;
    QString message;
};

class QwenVisionQtClient {
public:
    explicit QwenVisionQtClient(const AppConfig& config);

    VisionRecognitionResult recognizeForPrompt(const QString& imagePath,
                                               const QString& userPrompt) const;

private:
    QString readApiKey() const;
    VisionRecognitionResult postRecognition(const QByteArray& requestBody,
                                            const QString& imagePath) const;
    QString buildImageDataUrl(const QString& imagePath) const;

    const AppConfig& config_;
};
