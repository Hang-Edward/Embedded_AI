#pragma once

#include <QList>
#include <QString>

struct AgentUiMessage {
    QString role;
    QString title;
    QString rawText;
    QString htmlText;
    QString imagePath;
};

struct ArchivedChatSession {
    QString sessionId;
    QString title;
    QString summary;
    QString timestamp;
    QList<AgentUiMessage> messages;
};
