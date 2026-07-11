#pragma once

#include "ConnectionState.h"

#include <QList>
#include <QString>

struct ExternalConversationSyncDecision {
    QString newestKey;
    QList<ConversationRecord> pendingRecords;
};

class ExternalConversationSyncPolicy {
public:
    static QString recordKey(const ConversationRecord& record);
    static ExternalConversationSyncDecision decide(const QList<ConversationRecord>& recentRecords,
                                                   const QString& lastSeenKey,
                                                   bool appendNewestWhenUninitialized);
};
