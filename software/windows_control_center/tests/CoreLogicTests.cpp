#include "AgentWorkflowPolicy.h"
#include "ApiResponseParser.h"
#include "AppConfig.h"
#include "ChatInputPolicy.h"
#include "ChatSessionStore.h"
#include "MarkdownLatexRenderer.h"
#include "SystemSelfTest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest>

class CoreLogicTests final : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void enterAndShiftEnterPolicy();
    void sessionCreateSaveRestore();
    void sessionRejectsMalformedJson();
    void markdownLatexCodeAndTableRender();
    void compatibleApiResponses();
    void apiTimeoutEmptyAndNetworkFailures();
    void visualWorkflowFallback();
    void qwenToDeepSeekWorkflow();
    void sshHardwareProbeParsing();
    void selfTestReportSerialization();
};

void CoreLogicTests::enterAndShiftEnterPolicy() {
    QVERIFY(ChatInputPolicy::shouldSubmit(Qt::Key_Return, Qt::NoModifier, false));
    QVERIFY(ChatInputPolicy::shouldSubmit(Qt::Key_Enter, Qt::ControlModifier, false));
    QVERIFY(!ChatInputPolicy::shouldSubmit(Qt::Key_Return, Qt::ShiftModifier, false));
    QVERIFY(!ChatInputPolicy::shouldSubmit(Qt::Key_A, Qt::NoModifier, false));
    QVERIFY(!ChatInputPolicy::shouldSubmit(Qt::Key_Return, Qt::NoModifier, true));
}

void CoreLogicTests::sessionCreateSaveRestore() {
    ArchivedChatSession session;
    session.sessionId = QStringLiteral("session-001");
    session.title = QStringLiteral("视觉问答");
    session.summary = QStringLiteral("识别实验台并给出建议");
    session.timestamp = QStringLiteral("2026-07-08T12:00:00+08:00");
    session.messages = {
        {QStringLiteral("user"), QStringLiteral("我的需求"), QStringLiteral("解释当前画面"), {}, QStringLiteral("frame.jpg")},
        {QStringLiteral("assistant"), QStringLiteral("Agent 回复"), QStringLiteral("画面中是一张实验台。"), {}, {}}
    };

    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString filePath = directory.filePath(QStringLiteral("sessions.json"));
    QString error;
    QVERIFY2(ChatSessionStore::save(filePath, {session}, &error), qPrintable(error));
    const QList<ArchivedChatSession> restored = ChatSessionStore::load(filePath, &error);
    QVERIFY2(error.isEmpty(), qPrintable(error));
    QCOMPARE(restored.size(), 1);
    QCOMPARE(restored.first().sessionId, session.sessionId);
    QCOMPARE(restored.first().messages.size(), 2);
    QCOMPARE(restored.first().messages.first().imagePath, QStringLiteral("frame.jpg"));
}

void CoreLogicTests::sessionRejectsMalformedJson() {
    QString error;
    const QList<ArchivedChatSession> sessions = ChatSessionStore::deserialize("{broken", &error);
    QVERIFY(sessions.isEmpty());
    QVERIFY(error.contains(QStringLiteral("解析失败")));
}

void CoreLogicTests::markdownLatexCodeAndTableRender() {
    qputenv("EMBEDDED_AI_OFFLINE_LATEX", "1");
    AppConfig config;
    MarkdownLatexRenderer renderer(config);
    const QString html = renderer.renderToHtml(QStringLiteral(
        "# 标题\n\n- 列表项\n\n| 列 A | 列 B |\n|---|---|\n| 1 | 2 |\n\n"
        "```cpp\nint answer = 42;\n```\n\n####\n行内公式 $x^2+y^2$。\n\n"
        "$$v_1=v_2\\text{或}v_1=-v_2$$"));
    QVERIFY(html.contains(QStringLiteral("标题")));
    QVERIFY(html.contains(QStringLiteral("列表项")));
    QVERIFY(html.contains(QStringLiteral("<table"), Qt::CaseInsensitive));
    QVERIFY(html.contains(QStringLiteral("int answer = 42")));
    QVERIFY(html.contains(QStringLiteral("x^2+y^2")));
    QVERIFY(html.contains(QStringLiteral("或")));
    QVERIFY(!html.contains(QStringLiteral("####")));
}

void CoreLogicTests::compatibleApiResponses() {
    const ParsedApiResponse deepSeek = ApiResponseParser::parseDeepSeek(
        R"({"choices":[{"message":{"content":"SELF_TEST_OK"}}]})");
    QVERIFY(deepSeek.success);
    QCOMPARE(deepSeek.content, QStringLiteral("SELF_TEST_OK"));

    const ParsedApiResponse qwen = ApiResponseParser::parseQwen(
        R"({"choices":[{"message":{"content":[{"type":"text","text":"画面正常"}]}}]})");
    QVERIFY(qwen.success);
    QCOMPARE(qwen.content, QStringLiteral("画面正常"));
}

void CoreLogicTests::apiTimeoutEmptyAndNetworkFailures() {
    const ParsedApiResponse timeout = ApiResponseParser::parseDeepSeek({}, QStringLiteral("Connection timed out"));
    QVERIFY(!timeout.success);
    QVERIFY(timeout.message.contains(QStringLiteral("timed out")));

    const ParsedApiResponse interrupted = ApiResponseParser::parseQwen({}, QStringLiteral("Remote host closed the connection"));
    QVERIFY(!interrupted.success);
    QVERIFY(interrupted.message.contains(QStringLiteral("closed")));

    const ParsedApiResponse empty = ApiResponseParser::parseDeepSeek(
        R"({"choices":[{"message":{"content":""}}]})");
    QVERIFY(!empty.success);
    QVERIFY(empty.message.contains(QStringLiteral("没有有效内容")));
}

void CoreLogicTests::visualWorkflowFallback() {
    const VisionWorkflowDecision textOnly = AgentWorkflowPolicy::decideVisionStep(false, false, false);
    QVERIFY(!textOnly.useVisualContext);
    QVERIFY(textOnly.warningText.isEmpty());

    const VisionWorkflowDecision missingImage = AgentWorkflowPolicy::decideVisionStep(true, false, false);
    QVERIFY(!missingImage.useVisualContext);
    QVERIFY(missingImage.warningText.contains(QStringLiteral("回退")));

    const VisionWorkflowDecision qwenFailed = AgentWorkflowPolicy::decideVisionStep(
        true, true, false, QStringLiteral("API 超时"));
    QVERIFY(!qwenFailed.useVisualContext);
    QVERIFY(qwenFailed.warningText.contains(QStringLiteral("API 超时")));

    const VisionWorkflowDecision success = AgentWorkflowPolicy::decideVisionStep(true, true, true);
    QVERIFY(success.useVisualContext);
    QVERIFY(success.warningText.isEmpty());
}

void CoreLogicTests::qwenToDeepSeekWorkflow() {
    QStringList calls;
    const AgentWorkflowExecution normal = AgentWorkflowPolicy::execute(
        true,
        true,
        [&calls]() {
            calls << QStringLiteral("qwen");
            return AgentStageResult {true, QStringLiteral("桌面上有一道数学题"), {}};
        },
        [&calls](const QString& visualContext) {
            calls << QStringLiteral("deepseek:%1").arg(visualContext);
            return AgentStageResult {true, QStringLiteral("答案是 42"), {}};
        });
    QVERIFY(normal.success);
    QCOMPARE(calls, QStringList({QStringLiteral("qwen"), QStringLiteral("deepseek:桌面上有一道数学题")}));
    QCOMPARE(normal.answer, QStringLiteral("答案是 42"));

    calls.clear();
    const AgentWorkflowExecution fallback = AgentWorkflowPolicy::execute(
        true,
        true,
        [&calls]() {
            calls << QStringLiteral("qwen");
            return AgentStageResult {false, {}, QStringLiteral("网络中断")};
        },
        [&calls](const QString& visualContext) {
            calls << QStringLiteral("deepseek:%1").arg(visualContext);
            return AgentStageResult {true, QStringLiteral("按纯文本继续回答"), {}};
        });
    QVERIFY(fallback.success);
    QCOMPARE(calls, QStringList({QStringLiteral("qwen"), QStringLiteral("deepseek:")}));
    QVERIFY(fallback.warningText.contains(QStringLiteral("网络中断")));

    const AgentWorkflowExecution textFailure = AgentWorkflowPolicy::execute(
        false,
        false,
        {},
        [](const QString&) { return AgentStageResult {false, {}, QStringLiteral("响应超时")}; });
    QVERIFY(!textFailure.success);
    QCOMPARE(textFailure.errorText, QStringLiteral("响应超时"));
}

void CoreLogicTests::sshHardwareProbeParsing() {
    const QString output = QStringLiteral(
        "__SERVICE__\nactive\n"
        "__SERIAL__\n/dev/ttyACM1\n"
        "__NUCLEO__\nOK\n"
        "__VIDEO__\nOK\n");
    QCOMPARE(SelfTestProbeParser::sectionValue(output, QStringLiteral("SERVICE"), QStringLiteral("SERIAL")),
             QStringLiteral("active"));
    QCOMPARE(SelfTestProbeParser::sectionValue(output, QStringLiteral("SERIAL"), QStringLiteral("NUCLEO")),
             QStringLiteral("/dev/ttyACM1"));
    QCOMPARE(SelfTestProbeParser::sectionValue(output, QStringLiteral("NUCLEO"), QStringLiteral("VIDEO")),
             QStringLiteral("OK"));
}

void CoreLogicTests::selfTestReportSerialization() {
    SelfTestReport report;
    report.reportId = QStringLiteral("report-1");
    report.host = QStringLiteral("172.20.10.6");
    report.checks = {
        {QStringLiteral("ssh"), QStringLiteral("SSH 握手"), SelfTestOutcome::Passed,
         QStringLiteral("握手成功"), QStringLiteral("无需修复"), 12},
        {QStringLiteral("camera"), QStringLiteral("摄像头"), SelfTestOutcome::Warning,
         QStringLiteral("尚无最近画面"), QStringLiteral("拍摄一次后重试"), 4}
    };
    QCOMPARE(report.overallOutcome(), SelfTestOutcome::Warning);
    QVERIFY(report.summary().contains(QStringLiteral("2 项")));
    const QJsonDocument document = QJsonDocument::fromJson(report.toJson());
    QVERIFY(document.isObject());
    QCOMPARE(document.object().value(QStringLiteral("host")).toString(), QStringLiteral("172.20.10.6"));
    QVERIFY(report.toText().contains(QStringLiteral("拍摄一次后重试")));
}

QTEST_APPLESS_MAIN(CoreLogicTests)
#include "CoreLogicTests.moc"
