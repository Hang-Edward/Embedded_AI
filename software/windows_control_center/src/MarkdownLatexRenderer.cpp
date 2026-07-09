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
#include <QFuture>
#include <QThread>
#include <QThreadPool>
#include <QtConcurrent>

namespace {

struct PlaceholderItem {
    QString token;
    QString latex;
    bool blockMode = false;
};

struct PreparedFormula {
    QString latex;
    QString annotation;
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
    return QStringLiteral("EMBEDDEDAILATEX%1TOKEN").arg(index);
}

PreparedFormula prepareFormula(QString latex) {
    QStringList annotations;
    const QRegularExpression textWithCjk(
        QStringLiteral(R"(\\text\s*\{([^{}]*[\x{3400}-\x{9FFF}][^{}]*)\})"));
    while (true) {
        const QRegularExpressionMatch match = textWithCjk.match(latex);
        if (!match.hasMatch()) {
            break;
        }
        annotations << match.captured(1).trimmed();
        latex.replace(match.capturedStart(0), match.capturedLength(0), QStringLiteral("\\quad "));
    }

    // CodeCogs 的数学字体不包含中文。把残余中文保留为公式后的普通文本，
    // 避免服务端把它们渲染成 ####，同时不丢失原始说明。
    const QRegularExpression cjkRun(QStringLiteral(R"([\x{3400}-\x{9FFF}]+)"));
    int offset = 0;
    while (true) {
        const QRegularExpressionMatch match = cjkRun.match(latex, offset);
        if (!match.hasMatch()) {
            break;
        }
        annotations << match.captured(0);
        latex.replace(match.capturedStart(0), match.capturedLength(0), QStringLiteral("\\quad "));
        offset = match.capturedStart(0) + 6;
    }
    annotations.removeAll(QString());
    annotations.removeDuplicates();
    return {latex.trimmed(), annotations.join(QStringLiteral("；"))};
}

} // namespace

MarkdownLatexRenderer::MarkdownLatexRenderer(const AppConfig& config)
    : config_(config) {
}

QString MarkdownLatexRenderer::renderToHtml(const QString& markdownText) const {
    QString working = markdownText;
    working.replace(QRegularExpression(QStringLiteral(R"((?m)^(#{1,6})([^#\s]))")),
                    QStringLiteral("\\1 \\2"));
    working.replace(QRegularExpression(QStringLiteral(R"((?m)^#{2,6}\s*$)")), QString());
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

    QList<QFuture<QString>> formulaJobs;
    formulaJobs.reserve(placeholders.size());
    QThreadPool formulaPool;
    // 公式服务属于网络 I/O，有限并发可以缩短长回答排版时间，同时避免抢占 UI 和网络资源。
    formulaPool.setMaxThreadCount(qBound(1, QThread::idealThreadCount(), 4));
    for (const PlaceholderItem& item : placeholders) {
        formulaJobs.append(QtConcurrent::run(&formulaPool, [this, item]() {
            return renderFormulaToImageHtml(item.latex, item.blockMode);
        }));
    }
    for (int index = 0; index < placeholders.size(); ++index) {
        html.replace(placeholders[index].token, formulaJobs[index].result());
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
    const PreparedFormula prepared = prepareFormula(latex);
    const QString annotationHtml = prepared.annotation.isEmpty()
        ? QString()
        : QStringLiteral("<span style='color:#dcecff;font-family:\"Microsoft YaHei UI\",sans-serif;'>%1</span>")
              .arg(escapeHtml(prepared.annotation));
    const QString filePath = prepared.latex.isEmpty()
        ? QString()
        : downloadFormulaPng(prepared.latex, blockMode);
    if (QFile::exists(filePath)) {
        const QString localUrl = QUrl::fromLocalFile(filePath).toString();
        if (blockMode) {
            return QStringLiteral(
                       "<div style='text-align:center;margin:10px 0 8px 0;'>"
                       "<img src=\"%1\" style='max-width:82%%;max-height:7.2em;height:auto;display:inline-block;' />"
                       "</div>%2")
                .arg(localUrl,
                     annotationHtml.isEmpty()
                         ? QString()
                         : QStringLiteral("<div style='text-align:center;margin:-4px 0 10px 0;'>%1</div>")
                               .arg(annotationHtml));
        }
        return QStringLiteral(
                   "<img src=\"%1\" style='vertical-align:-0.12em;max-height:1.18em;width:auto;' />%2")
            .arg(localUrl, annotationHtml);
    }

    const QString fallback = QStringLiteral(
        "<span style='font-family:Consolas,monospace;color:#f4f8ff;background:rgba(255,255,255,0.06);"
        "padding:2px 6px;border-radius:6px;'>%1</span>")
        .arg(escapeHtml(prepared.latex));
    if (blockMode) {
        return QStringLiteral("<div style='text-align:center;margin:10px 0;'>%1</div>%2")
            .arg(prepared.latex.isEmpty() ? QString() : fallback,
                 annotationHtml.isEmpty()
                     ? QString()
                     : QStringLiteral("<div style='text-align:center;margin:-4px 0 10px 0;'>%1</div>")
                           .arg(annotationHtml));
    }
    return (prepared.latex.isEmpty() ? QString() : fallback) + annotationHtml;
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
