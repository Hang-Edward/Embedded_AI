#include "DeepSeekChatClient.h"

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
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

QString defaultSystemPrompt() {
    return QStringLiteral(
        "你是 Embedded AI Reality Bridge 的主智能体。"
        "你负责和用户对话、理解任务、整合视觉观察，并给出最终回答。"
        "如果给了视觉观察，那只是来自视觉模型的客观识别结果，不是最终答案。"
        "最终回答必须由你独立组织。"
        "请尽量使用清晰中文，必要时使用 Markdown、列表、公式。"
        "如果用户在问解题、分析、规划、调试，你要像一个真正的 agent 一样先理解，再回答。");
}

} // namespace

DeepSeekChatClient::DeepSeekChatClient(const AppConfig& config)
    : config_(config) {
}

ChatCompletionResult DeepSeekChatClient::complete(const QList<ChatCompletionMessage>& messages,
                                                  const QString& visualContext) const {
    const QString apiKey = readApiKey();
    if (apiKey.isEmpty()) {
        return {false, {}, QStringLiteral("未找到 DeepSeek API key。请准备 %1 或设置环境变量 %2。")
                                 .arg(config_.deepSeekApiKeyFile, config_.deepSeekApiKeyEnv)};
    }

    QJsonArray jsonMessages;
    jsonMessages.append(QJsonObject {
        {"role", "system"},
        {"content", defaultSystemPrompt()}
    });

    if (!visualContext.trimmed().isEmpty()) {
        jsonMessages.append(QJsonObject {
            {"role", "system"},
            {"content", QStringLiteral("以下是视觉模型提供的客观观察结果，仅供推理参考：\n%1").arg(visualContext)}
        });
    }

    for (const ChatCompletionMessage& item : messages) {
        jsonMessages.append(QJsonObject {
            {"role", item.role},
            {"content", item.content}
        });
    }

    QJsonObject root {
        {"model", config_.deepSeekModel},
        {"messages", jsonMessages},
        {"stream", false}
    };
    return postChat(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

QString DeepSeekChatClient::readApiKey() const {
    const QString fromFile = readSecretTrimmed(resolveSecretPath(config_.deepSeekApiKeyFile));
    if (!fromFile.isEmpty()) {
        return fromFile;
    }
    return QProcessEnvironment::systemEnvironment().value(config_.deepSeekApiKeyEnv).trimmed();
}

ChatCompletionResult DeepSeekChatClient::postChat(const QByteArray& requestBody) const {
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(endpointFor(config_.deepSeekBaseUrl)));
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
        return {false, {}, QStringLiteral("DeepSeek 请求超时。")};
    }

    const QByteArray body = reply->readAll();
    const QString networkError = reply->error() == QNetworkReply::NoError
        ? QString()
        : reply->errorString();
    reply->deleteLater();

    const QJsonDocument json = QJsonDocument::fromJson(body);
    if (!json.isObject()) {
        return {false, {}, networkError.isEmpty() ? QStringLiteral("DeepSeek 返回了无法解析的响应。") : networkError};
    }

    const QJsonObject root = json.object();
    if (root.contains("error")) {
        const QJsonObject errorObject = root.value("error").toObject();
        return {false, {}, errorObject.value("message").toString(QStringLiteral("DeepSeek 返回错误。"))};
    }

    const QJsonArray choices = root.value("choices").toArray();
    if (choices.isEmpty()) {
        return {false, {}, QStringLiteral("DeepSeek 响应里没有 choices。")};
    }

    const QJsonObject message = choices.first().toObject().value("message").toObject();
    const QString content = message.value("content").toString().trimmed();
    if (content.isEmpty()) {
        return {false, {}, QStringLiteral("DeepSeek 响应里没有有效内容。")};
    }
    return {true, content, QStringLiteral("DeepSeek 回复成功。")};
}
