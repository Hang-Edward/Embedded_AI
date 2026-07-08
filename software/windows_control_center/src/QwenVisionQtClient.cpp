#include "QwenVisionQtClient.h"

#include "ApiResponseParser.h"

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QBuffer>
#include <QImage>
#include <QImageReader>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcessEnvironment>
#include <QTimer>
#include <QUrl>

namespace {

QString readSecretTrimmed(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll()).trimmed();
}

QString resolveSecretPath(const QString& rawPath) {
    const QString trimmed = rawPath.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    const QFileInfo directInfo(trimmed);
    if (directInfo.isAbsolute() && directInfo.exists()) {
        return directInfo.absoluteFilePath();
    }
    if (directInfo.exists()) {
        return directInfo.absoluteFilePath();
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidates = {
        QDir(appDir).filePath(trimmed),
        QDir(appDir).filePath(QStringLiteral("../../") + trimmed),
        QDir(appDir).filePath(QStringLiteral("../../../") + trimmed)
    };

    for (const QString& candidate : candidates) {
        const QFileInfo info(QDir::cleanPath(candidate));
        if (info.exists()) {
            return info.absoluteFilePath();
        }
    }

    return trimmed;
}

QString endpointFor(const QString& baseUrl) {
    QString normalized = baseUrl.trimmed();
    while (normalized.endsWith('/')) {
        normalized.chop(1);
    }
    return normalized + "/chat/completions";
}

QString qwenPromptFor(const QString& userPrompt) {
    return QStringLiteral(
               "请只做视觉识别，不要替用户决策，也不要代替主模型回答。"
               "请客观描述当前画面中与用户需求相关的内容，尽量提取可见对象、文字、公式、布局、题目、设备状态。"
               "输出中文短报告。\n\n用户当前需求：%1")
        .arg(userPrompt);
}

} // namespace

QwenVisionQtClient::QwenVisionQtClient(const AppConfig& config)
    : config_(config) {
}

VisionRecognitionResult QwenVisionQtClient::recognizeForPrompt(const QString& imagePath,
                                                               const QString& userPrompt) const {
    const QString apiKey = readApiKey();
    if (apiKey.isEmpty()) {
        return {false, {}, QStringLiteral("未找到 Qwen 视觉 API key。")};
    }

    const QString imageUrl = buildImageDataUrl(imagePath);
    if (imageUrl.isEmpty()) {
        return {false, {}, QStringLiteral("无法读取当前图片，不能做视觉识别。")};
    }

    QJsonArray content;
    content.append(QJsonObject {
        {"type", "image_url"},
        {"image_url", QJsonObject {{"url", imageUrl}}}
    });
    content.append(QJsonObject {
        {"type", "text"},
        {"text", qwenPromptFor(userPrompt)}
    });

    QJsonArray messages;
    messages.append(QJsonObject {
        {"role", "user"},
        {"content", content}
    });

    QJsonObject root {
        {"model", config_.qwenVisionModel},
        {"messages", messages}
    };

    return postRecognition(QJsonDocument(root).toJson(QJsonDocument::Compact), imagePath);
}

QString QwenVisionQtClient::readApiKey() const {
    const QString fromFile = readSecretTrimmed(resolveSecretPath(config_.qwenVisionApiKeyFile));
    if (!fromFile.isEmpty()) {
        return fromFile;
    }
    return QProcessEnvironment::systemEnvironment().value(config_.qwenVisionApiKeyEnv).trimmed();
}

VisionRecognitionResult QwenVisionQtClient::postRecognition(const QByteArray& requestBody,
                                                            const QString& imagePath) const {
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(endpointFor(config_.qwenVisionBaseUrl)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QByteArray("Bearer ") + readApiKey().toUtf8());

    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    timeout.start(45000);

    QNetworkReply* reply = manager.post(request, requestBody);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    loop.exec();

    if (timeout.isActive() == false && reply->isFinished() == false) {
        reply->abort();
        reply->deleteLater();
        return {false, {}, QStringLiteral("Qwen 视觉请求超时。")};
    }

    const QByteArray body = reply->readAll();
    const QString networkError = reply->error() == QNetworkReply::NoError
        ? QString()
        : reply->errorString();
    reply->deleteLater();

    const ParsedApiResponse parsed = ApiResponseParser::parseQwen(body, networkError);
    if (!parsed.success && parsed.message.contains(QStringLiteral("没有有效内容"))) {
        return {false, {}, QStringLiteral("%1 图像：%2").arg(parsed.message, imagePath)};
    }
    return {parsed.success, parsed.content, parsed.message};
}

QString QwenVisionQtClient::buildImageDataUrl(const QString& imagePath) const {
    QImageReader reader(imagePath);
    reader.setAutoTransform(true);
    const QImage original = reader.read();
    if (original.isNull()) {
        return {};
    }

    // 中文注释：视觉识别前先把图片压到适中的边长和质量，明显减少 base64 体积，
    // 对“结合当前画面”的首包延迟帮助很大。
    QImage prepared = original;
    constexpr int kMaxEdge = 960;
    if (prepared.width() > kMaxEdge || prepared.height() > kMaxEdge) {
        prepared = prepared.scaled(kMaxEdge,
                                   kMaxEdge,
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
    }

    QByteArray bytes;
    QBuffer buffer(&bytes);
    if (!buffer.open(QIODevice::WriteOnly)) {
        return {};
    }
    if (!prepared.save(&buffer, "JPEG", 76)) {
        return {};
    }
    if (bytes.isEmpty()) {
        return {};
    }
    return QStringLiteral("data:%1;base64,%2")
        .arg(QStringLiteral("image/jpeg"), QString::fromLatin1(bytes.toBase64()));
}
