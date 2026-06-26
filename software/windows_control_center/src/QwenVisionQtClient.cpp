#include "QwenVisionQtClient.h"

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
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

    const QJsonDocument json = QJsonDocument::fromJson(body);
    if (!json.isObject()) {
        return {false, {}, networkError.isEmpty() ? QStringLiteral("Qwen 视觉返回了无法解析的响应。") : networkError};
    }

    const QJsonObject root = json.object();
    if (root.contains("error")) {
        const QJsonObject errorObject = root.value("error").toObject();
        return {false, {}, errorObject.value("message").toString(QStringLiteral("Qwen 视觉返回错误。"))};
    }

    const QJsonArray choices = root.value("choices").toArray();
    if (choices.isEmpty()) {
        return {false, {}, QStringLiteral("Qwen 视觉响应里没有 choices。")};
    }

    const QJsonObject message = choices.first().toObject().value("message").toObject();
    const QString content = message.value("content").toString().trimmed();
    if (!content.isEmpty()) {
        return {true, content, QStringLiteral("Qwen 视觉识别成功。")};
    }

    const QJsonArray contentArray = message.value("content").toArray();
    QStringList textParts;
    for (const QJsonValue& value : contentArray) {
        const QJsonObject object = value.toObject();
        if (object.value("type").toString() == "text") {
            textParts << object.value("text").toString();
        }
    }
    const QString merged = textParts.join("\n").trimmed();
    if (merged.isEmpty()) {
        return {false, {}, QStringLiteral("Qwen 视觉没有返回有效文本。图像：%1").arg(imagePath)};
    }
    return {true, merged, QStringLiteral("Qwen 视觉识别成功。")};
}

QString QwenVisionQtClient::buildImageDataUrl(const QString& imagePath) const {
    QFile file(imagePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    const QByteArray bytes = file.readAll();
    if (bytes.isEmpty()) {
        return {};
    }

    QImageReader reader(imagePath);
    const QByteArray format = reader.format().toLower();
    const QString mime = format == "png" ? "image/png" : "image/jpeg";
    return QStringLiteral("data:%1;base64,%2")
        .arg(mime, QString::fromLatin1(bytes.toBase64()));
}
