#include "ApiResponseParser.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

ParsedApiResponse ApiResponseParser::parseDeepSeek(const QByteArray& body,
                                                   const QString& networkError) {
    return parseOpenAiCompatible(body, networkError, QStringLiteral("DeepSeek"));
}

ParsedApiResponse ApiResponseParser::parseQwen(const QByteArray& body,
                                               const QString& networkError) {
    return parseOpenAiCompatible(body, networkError, QStringLiteral("Qwen 视觉"));
}

ParsedApiResponse ApiResponseParser::parseOpenAiCompatible(const QByteArray& body,
                                                           const QString& networkError,
                                                           const QString& providerName) {
    if (!networkError.trimmed().isEmpty() && body.trimmed().isEmpty()) {
        return {false, {}, QStringLiteral("%1 网络请求失败：%2").arg(providerName, networkError)};
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);
    if (!document.isObject()) {
        return {false,
                {},
                networkError.trimmed().isEmpty()
                    ? QStringLiteral("%1 返回了无法解析的响应：%2")
                          .arg(providerName, parseError.errorString())
                    : QStringLiteral("%1 请求失败：%2").arg(providerName, networkError)};
    }

    const QJsonObject root = document.object();
    if (root.contains("error")) {
        const QJsonObject errorObject = root.value("error").toObject();
        return {false,
                {},
                errorObject.value("message").toString(
                    QStringLiteral("%1 返回错误。").arg(providerName))};
    }

    const QJsonArray choices = root.value("choices").toArray();
    if (choices.isEmpty()) {
        return {false, {}, QStringLiteral("%1 响应里没有 choices。").arg(providerName)};
    }

    const QJsonObject message = choices.first().toObject().value("message").toObject();
    QString content = message.value("content").toString().trimmed();
    if (content.isEmpty()) {
        QStringList parts;
        for (const QJsonValue& value : message.value("content").toArray()) {
            const QJsonObject item = value.toObject();
            if (item.value("type").toString() == "text") {
                parts << item.value("text").toString();
            }
        }
        content = parts.join('\n').trimmed();
    }
    if (content.isEmpty()) {
        return {false, {}, QStringLiteral("%1 响应里没有有效内容。").arg(providerName)};
    }
    return {true, content, QStringLiteral("%1 响应解析成功。").arg(providerName)};
}
