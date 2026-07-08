#include "ChatSessionStore.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

QByteArray ChatSessionStore::serialize(const QList<ArchivedChatSession>& sessions) {
    QJsonArray sessionsArray;
    for (const ArchivedChatSession& session : sessions) {
        QJsonArray items;
        for (const AgentUiMessage& message : session.messages) {
            items.append(QJsonObject {
                {"role", message.role},
                {"title", message.title},
                {"rawText", message.rawText},
                {"imagePath", message.imagePath}
            });
        }
        sessionsArray.append(QJsonObject {
            {"sessionId", session.sessionId},
            {"title", session.title},
            {"summary", session.summary},
            {"timestamp", session.timestamp},
            {"messages", items}
        });
    }
    return QJsonDocument(sessionsArray).toJson(QJsonDocument::Indented);
}

QList<ArchivedChatSession> ChatSessionStore::deserialize(const QByteArray& data,
                                                          QString* error) {
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
    if (!document.isArray()) {
        if (error != nullptr) {
            *error = parseError.error == QJsonParseError::NoError
                ? QStringLiteral("会话文件根节点不是数组。")
                : QStringLiteral("会话 JSON 解析失败：%1").arg(parseError.errorString());
        }
        return {};
    }

    QList<ArchivedChatSession> sessions;
    for (const QJsonValue& value : document.array()) {
        if (!value.isObject()) {
            continue;
        }
        const QJsonObject object = value.toObject();
        ArchivedChatSession session;
        session.sessionId = object.value("sessionId").toString().trimmed();
        session.title = object.value("title").toString().trimmed();
        session.summary = object.value("summary").toString().trimmed();
        session.timestamp = object.value("timestamp").toString().trimmed();

        for (const QJsonValue& messageValue : object.value("messages").toArray()) {
            if (!messageValue.isObject()) {
                continue;
            }
            const QJsonObject messageObject = messageValue.toObject();
            const QString role = messageObject.value("role").toString().trimmed();
            const QString rawText = messageObject.value("rawText").toString();
            if (role.isEmpty() || rawText.trimmed().isEmpty()) {
                continue;
            }
            session.messages.append({role,
                                     messageObject.value("title").toString().trimmed(),
                                     rawText,
                                     QString(),
                                     messageObject.value("imagePath").toString()});
        }

        if (!session.sessionId.isEmpty() && !session.messages.isEmpty()) {
            sessions.append(session);
        }
    }
    if (error != nullptr) {
        error->clear();
    }
    return sessions;
}

bool ChatSessionStore::save(const QString& filePath,
                            const QList<ArchivedChatSession>& sessions,
                            QString* error) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error != nullptr) {
            *error = QStringLiteral("无法写入会话文件：%1").arg(file.errorString());
        }
        return false;
    }
    const QByteArray data = serialize(sessions);
    if (file.write(data) != data.size()) {
        if (error != nullptr) {
            *error = QStringLiteral("会话文件写入不完整：%1").arg(file.errorString());
        }
        return false;
    }
    if (error != nullptr) {
        error->clear();
    }
    return true;
}

QList<ArchivedChatSession> ChatSessionStore::load(const QString& filePath,
                                                  QString* error) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error != nullptr) {
            *error = QStringLiteral("无法读取会话文件：%1").arg(file.errorString());
        }
        return {};
    }
    return deserialize(file.readAll(), error);
}
