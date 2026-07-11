#include "ExternalConversationSyncPolicy.h"

#include <QCryptographicHash>

QString ExternalConversationSyncPolicy::recordKey(const ConversationRecord& record) {
    const QByteArray material = QStringLiteral("%1|%2|%3|%4")
        .arg(record.title.trimmed(),
             record.timestamp.trimmed(),
             record.userText.trimmed(),
             record.aiText.trimmed())
        .toUtf8();
    return QString::fromLatin1(QCryptographicHash::hash(material, QCryptographicHash::Sha256).toHex());
}

ExternalConversationSyncDecision ExternalConversationSyncPolicy::decide(
    const QList<ConversationRecord>& recentRecords,
    const QString& lastSeenKey,
    bool appendNewestWhenUninitialized) {
    ExternalConversationSyncDecision decision;
    if (recentRecords.isEmpty()) {
        return decision;
    }

    decision.newestKey = recordKey(recentRecords.first());
    if (lastSeenKey.isEmpty()) {
        if (appendNewestWhenUninitialized && !recentRecords.first().aiText.trimmed().isEmpty()) {
            decision.pendingRecords.append(recentRecords.first());
        }
        return decision;
    }
    if (decision.newestKey == lastSeenKey) {
        return decision;
    }

    for (const ConversationRecord& record : recentRecords) {
        if (recordKey(record) == lastSeenKey) {
            break;
        }
        if (!record.aiText.trimmed().isEmpty()) {
            decision.pendingRecords.prepend(record);
        }
    }
    return decision;
}
