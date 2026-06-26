#pragma once

#include "AppConfig.h"

#include <QString>
#include <QList>

struct ChatCompletionMessage {
    QString role;
    QString content;
};

struct ChatCompletionResult {
    bool success = false;
    QString content;
    QString message;
};

class DeepSeekChatClient {
public:
    explicit DeepSeekChatClient(const AppConfig& config);

    ChatCompletionResult complete(const QList<ChatCompletionMessage>& messages,
                                  const QString& visualContext = QString()) const;

private:
    QString readApiKey() const;
    ChatCompletionResult postChat(const QByteArray& requestBody) const;

    const AppConfig& config_;
};
