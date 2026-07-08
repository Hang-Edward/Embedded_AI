#pragma once

#include <QByteArray>
#include <QString>

struct ParsedApiResponse {
    bool success = false;
    QString content;
    QString message;
};

class ApiResponseParser {
public:
    static ParsedApiResponse parseDeepSeek(const QByteArray& body,
                                           const QString& networkError = QString());
    static ParsedApiResponse parseQwen(const QByteArray& body,
                                       const QString& networkError = QString());

private:
    static ParsedApiResponse parseOpenAiCompatible(const QByteArray& body,
                                                   const QString& networkError,
                                                   const QString& providerName);
};
