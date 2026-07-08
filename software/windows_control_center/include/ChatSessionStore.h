#pragma once

#include "ChatSessionModels.h"

#include <QList>
#include <QString>

class ChatSessionStore {
public:
    static QByteArray serialize(const QList<ArchivedChatSession>& sessions);
    static QList<ArchivedChatSession> deserialize(const QByteArray& data,
                                                   QString* error = nullptr);
    static bool save(const QString& filePath,
                     const QList<ArchivedChatSession>& sessions,
                     QString* error = nullptr);
    static QList<ArchivedChatSession> load(const QString& filePath,
                                           QString* error = nullptr);
};
