#include "MarkdownLatexRenderer.h"

#include <QCryptographicHash>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextDocument>
#include <QTimer>
#include <QUrl>

namespace {

struct PlaceholderItem {
    QString token;
    QString latex;
    bool blockMode = false;
};

QString fetchUrlToFile(const QUrl& url, const QString& outputPath) {
    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    timeout.start(20000);
    QNetworkReply* reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    loop.exec();

    if (timeout.isActive() == false && reply->isFinished() == false) {
        reply->abort();
    }

    if (reply->error() != QNetworkReply::NoError) {
        const QString error = reply->errorString();
        reply->deleteLater();
        return error;
    }

    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly)) {
        reply->deleteLater();
        return QStringLiteral("无法写入公式缓存文件。");
    }
    file.write(reply->readAll());
    file.close();
    reply->deleteLater();
    return {};
}

QString escapeHtml(const QString& text) {
    QString escaped = text.toHtmlEscaped();
    escaped.replace('\n', "<br/>");
    return escaped;
}

QString placeholderToken(int index) {
    return QStringLiteral("EMBEDDED_AI_LATEX_%1_TOKEN").arg(index);
}

} // namespace

MarkdownLatexRenderer::MarkdownLatexRenderer(const AppConfig& config)
    : config_(config) {
}

QString MarkdownLatexRenderer::renderToHtml(const QString& markdownText) const {
    QString working = markdownText;
    QList<PlaceholderItem> placeholders;

    auto extractByPattern = [&](const QRegularExpression& expression, bool blockMode) {
        int offset = 0;
        while (true) {
            const QRegularExpressionMatch match = expression.match(working, offset);
            if (!match.hasMatch()) {
                break;
            }
            const QString token = placeholderToken(placeholders.size());
            placeholders.push_back({token, match.captured(1).trimmed(), blockMode});
            working.replace(match.capturedStart(0), match.capturedLength(0), token);
            offset = working.indexOf(token, match.capturedStart(0)) + token.size();
        }
    };

    // 中文注释：先替换块级公式，再替换行内公式，避免正则互相吞掉。
    // 这里同时兼容 $...$ / $$...$$ 与 \( ... \) / \[ ... \] 两套常见写法。
    extractByPattern(QRegularExpression(R"(\$\$([\s\S]+?)\$\$)"), true);
    extractByPattern(QRegularExpression(R"(\\\[([\s\S]+?)\\\])"), true);
    extractByPattern(QRegularExpression(R"((?<!\\)\$([^\n$]+?)(?<!\\)\$)"), false);
    extractByPattern(QRegularExpression(R"(\\\(([\s\S]+?)\\\))"), false);

    QTextDocument document;
    document.setMarkdown(working);
    QString html = document.toHtml();

    for (const PlaceholderItem& item : placeholders) {
        html.replace(item.token, renderFormulaToImageHtml(item.latex, item.blockMode));
    }

    html += QStringLiteral(
        "<style>"
        "body{font-family:'Microsoft YaHei UI','Segoe UI',sans-serif;color:#dcecff;}"
        "p,li{line-height:1.65;}"
        "pre{background:rgba(5,14,32,0.88);padding:12px;border-radius:12px;color:#eef6ff;}"
        "code{background:rgba(255,255,255,0.06);padding:2px 6px;border-radius:6px;}"
        "blockquote{border-left:3px solid rgba(118,184,255,0.55);margin-left:0;padding-left:12px;color:#c8ddf5;}"
        "table{border-collapse:collapse;margin:8px 0;}"
        "td,th{border:1px solid rgba(175,220,255,0.18);padding:6px 10px;}"
        "</style>");
    return html;
}

QString MarkdownLatexRenderer::renderFormulaToImageHtml(const QString& latex, bool blockMode) const {
    const QString filePath = downloadFormulaPng(latex, blockMode);
    if (QFile::exists(filePath)) {
        const QString localUrl = QUrl::fromLocalFile(filePath).toString();
        if (blockMode) {
            return QStringLiteral(
                       "<div style='text-align:center;margin:10px 0 8px 0;'>"
                       "<img src=\"%1\" style='max-width:82%%;max-height:7.2em;height:auto;display:inline-block;' />"
                       "</div>")
                .arg(localUrl);
        }
        return QStringLiteral(
                   "<img src=\"%1\" style='vertical-align:-0.12em;max-height:1.18em;width:auto;' />")
            .arg(localUrl);
    }

    const QString fallback = QStringLiteral(
        "<span style='font-family:Consolas,monospace;color:#f4f8ff;background:rgba(255,255,255,0.06);"
        "padding:2px 6px;border-radius:6px;'>%1</span>")
        .arg(escapeHtml(latex));
    if (blockMode) {
        return QStringLiteral("<div style='text-align:center;margin:10px 0;'>%1</div>").arg(fallback);
    }
    return fallback;
}

QString MarkdownLatexRenderer::downloadFormulaPng(const QString& latex, bool blockMode) const {
    if (qEnvironmentVariableIntValue("EMBEDDED_AI_OFFLINE_LATEX") != 0) {
        return {};
    }
    const QString hashSource = QStringLiteral("latex-style-v3|%1%2")
        .arg(blockMode ? QStringLiteral("block:") : QStringLiteral("inline:"),
             latex);
    const QString cacheKey = QString::fromLatin1(
        QCryptographicHash::hash(hashSource.toUtf8(),
                                 QCryptographicHash::Sha1)
            .toHex());
    const QString outputPath = QDir(cacheDirPath()).filePath(cacheKey + ".png");
    if (QFile::exists(outputPath)) {
        return outputPath;
    }

    const QString command = blockMode
        ? QStringLiteral("\\dpi{132}\\bg{transparent}\\fg{white}%1").arg(latex)
        : QStringLiteral("\\dpi{112}\\bg{transparent}\\fg{white}\\inline %1").arg(latex);
    const QUrl url(QStringLiteral("https://latex.codecogs.com/png.image?%1")
                       .arg(QString::fromLatin1(QUrl::toPercentEncoding(command))));
    const QString error = fetchUrlToFile(url, outputPath);
    if (!error.isEmpty()) {
        return error;
    }
    return outputPath;
}

QString MarkdownLatexRenderer::cacheDirPath() const {
    QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (baseDir.trimmed().isEmpty()) {
        baseDir = QDir::currentPath();
    }
    baseDir = QDir(baseDir).filePath("ui-math-cache");
    QDir().mkpath(baseDir);
    return baseDir;
}
