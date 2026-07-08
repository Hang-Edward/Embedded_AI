#include "HardwarePage.h"

#include "StatusCard.h"

#include <QFile>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

namespace {

HealthLevel assistantLevel(AssistantStatus status) {
    if (status == AssistantStatus::Ready) {
        return HealthLevel::Ok;
    }
    if (status == AssistantStatus::Listening || status == AssistantStatus::Thinking || status == AssistantStatus::Connecting) {
        return HealthLevel::Checking;
    }
    if (status == AssistantStatus::Warning) {
        return HealthLevel::Warning;
    }
    if (status == AssistantStatus::Error || status == AssistantStatus::Offline) {
        return HealthLevel::Error;
    }
    return HealthLevel::Unknown;
}

QString assistantDetail(const ConnectionState& state) {
    switch (state.assistantStatus) {
    case AssistantStatus::Ready:
        return "绿灯：系统就绪，可以按三键键盘 K-B 触发语音输入。";
    case AssistantStatus::Listening:
        return state.voiceCountdownSeconds > 0
            ? QString("黄灯：正在录音，还剩 %1 秒。").arg(state.voiceCountdownSeconds)
            : "黄灯：正在录音。";
    case AssistantStatus::Thinking:
        return "黄灯：AI 正在识别语音并分析画面。";
    case AssistantStatus::Warning:
        return "黄灯：系统可用，但存在需要检查的项目。";
    case AssistantStatus::Error:
        return "红灯：流程故障，请查看原始日志。";
    case AssistantStatus::Offline:
        return "红灯：树莓派服务离线。";
    case AssistantStatus::Connecting:
        return "正在检测树莓派网络、SSH 和服务状态。";
    }
    return "等待状态刷新。";
}

} // namespace

HardwarePage::HardwarePage(AppConfig& config, QWidget* parent)
    : BasePage("连接诊断", "一键验证网络、SSH、树莓派服务、NUCLEO、C270、模型 API、本地反馈和文件读写。", parent),
      config_(config),
      runner_(this) {
    auto* selfTestPanel = new QWidget(this);
    selfTestPanel->setObjectName(QStringLiteral("selfTestPanel"));
    selfTestPanel->setStyleSheet(QStringLiteral(
        "QWidget#selfTestPanel { background: rgba(7, 18, 42, 0.46); border: 1px solid rgba(159, 210, 255, 0.14); border-radius: 12px; }"
        "QTreeWidget { background: rgba(4, 13, 31, 0.58); border: 1px solid rgba(159, 210, 255, 0.12); border-radius: 10px; }"
        "QTreeWidget::item { min-height: 30px; padding: 5px; }"
        "QHeaderView::section { background: rgba(24, 54, 104, 0.82); color: #dcecff; border: none; padding: 8px; font-weight: 700; }"));
    auto* panelLayout = new QVBoxLayout(selfTestPanel);
    panelLayout->setContentsMargins(16, 14, 16, 16);
    panelLayout->setSpacing(10);

    auto* actions = new QHBoxLayout();
    actions->setContentsMargins(0, 0, 0, 0);
    auto* intro = new QLabel(QStringLiteral("完整链路验收\n会执行真实 Qwen 视觉调用和 DeepSeek 文本调用，通常需要 10–60 秒。"), selfTestPanel);
    intro->setWordWrap(true);
    runButton_ = new QPushButton(QStringLiteral("一键完整检查"), selfTestPanel);
    runButton_->setObjectName(QStringLiteral("primaryButton"));
    exportJsonButton_ = new QPushButton(QStringLiteral("导出 JSON"), selfTestPanel);
    exportJsonButton_->setObjectName(QStringLiteral("secondaryButton"));
    exportTextButton_ = new QPushButton(QStringLiteral("导出文本"), selfTestPanel);
    exportTextButton_->setObjectName(QStringLiteral("secondaryButton"));
    exportJsonButton_->setEnabled(false);
    exportTextButton_->setEnabled(false);
    actions->addWidget(intro, 1);
    actions->addWidget(runButton_);
    actions->addWidget(exportJsonButton_);
    actions->addWidget(exportTextButton_);

    progress_ = new QProgressBar(selfTestPanel);
    progress_->setRange(0, 13);
    progress_->setValue(0);
    progress_->setTextVisible(true);
    reportSummary_ = new QLabel(QStringLiteral("尚未执行完整检查。"), selfTestPanel);
    reportSummary_->setWordWrap(true);

    resultTree_ = new QTreeWidget(selfTestPanel);
    resultTree_->setColumnCount(5);
    resultTree_->setHeaderLabels({QStringLiteral("结果"), QStringLiteral("检查项"), QStringLiteral("详情"), QStringLiteral("修复建议"), QStringLiteral("耗时")});
    resultTree_->setRootIsDecorated(false);
    resultTree_->setAlternatingRowColors(false);
    resultTree_->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    resultTree_->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    resultTree_->header()->setSectionResizeMode(2, QHeaderView::Stretch);
    resultTree_->header()->setSectionResizeMode(3, QHeaderView::Stretch);
    resultTree_->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    resultTree_->setMinimumHeight(330);

    panelLayout->addLayout(actions);
    panelLayout->addWidget(progress_);
    panelLayout->addWidget(reportSummary_);
    panelLayout->addWidget(resultTree_);
    bodyLayout()->addWidget(selfTestPanel);

    auto* container = new QWidget(this);
    grid_ = new QGridLayout(container);
    grid_->setContentsMargins(0, 0, 0, 0);
    grid_->setSpacing(14);
    bodyLayout()->addWidget(container);

    QObject::connect(runButton_, &QPushButton::clicked, this, [this]() { startFullCheck(); });
    QObject::connect(exportJsonButton_, &QPushButton::clicked, this, [this]() { exportReport(true); });
    QObject::connect(exportTextButton_, &QPushButton::clicked, this, [this]() { exportReport(false); });
    runner_.setProgressCallback([this](const SelfTestCheck& check, int completed, int total) {
        progress_->setRange(0, total);
        progress_->setValue(completed);
        reportSummary_->setText(QStringLiteral("正在检查：%1（%2/%3）").arg(check.name).arg(completed).arg(total));
        updateCheckRow(check);
    });
    runner_.setFinishedCallback([this](const SelfTestReport& report) { finishFullCheck(report); });
}

void HardwarePage::setState(const ConnectionState& state) {
    currentState_ = state;
    QStringList keyParts;
    keyParts << QString::number(static_cast<int>(state.assistantStatus))
             << state.assistantStatusText
             << state.activeHost
             << (state.serviceActive ? "1" : "0")
             << state.localFramePath;
    for (const HealthItem& item : state.hardwareItems) {
        keyParts << item.name + "|" + item.detail + "|" + QString::number(static_cast<int>(item.level));
    }
    const QString newKey = keyParts.join("||");
    if (newKey == lastStateKey_) {
        return;
    }
    lastStateKey_ = newKey;

    while (auto* item = grid_->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    QList<HealthItem> displayItems;
    displayItems << HealthItem{"对话触发按键", assistantDetail(state), assistantLevel(state.assistantStatus)};
    displayItems << HealthItem{"树莓派网络", state.sshOnline ? "SSH 已连接：" + state.activeHost : "正在等待树莓派连接", state.sshOnline ? HealthLevel::Ok : HealthLevel::Checking};
    displayItems << HealthItem{"自启动服务", state.serviceActive ? "embedded-ai.service 正在运行" : "服务未确认运行", state.serviceActive ? HealthLevel::Ok : HealthLevel::Warning};
    displayItems << HealthItem{"最新画面", state.localFramePath.isEmpty() ? "尚未拉取摄像头图片" : "已缓存：" + state.localFramePath, state.localFramePath.isEmpty() ? HealthLevel::Warning : HealthLevel::Ok};

    for (const HealthItem& item : state.hardwareItems) {
        displayItems << item;
    }

    for (int i = 0; i < displayItems.size(); ++i) {
        const int row = i / 2;
        const int col = i % 2;
        grid_->addWidget(new StatusCard(displayItems[i].name, displayItems[i].detail, displayItems[i].level, this), row, col);
    }
}

void HardwarePage::startFullCheck() {
    if (runner_.isRunning()) {
        return;
    }
    resultTree_->clear();
    latestReport_ = {};
    progress_->setRange(0, 13);
    progress_->setValue(0);
    reportSummary_->setText(QStringLiteral("正在准备完整检查，请保持树莓派、NUCLEO 和 C270 连接。"));
    runButton_->setEnabled(false);
    runButton_->setText(QStringLiteral("检查进行中..."));
    exportJsonButton_->setEnabled(false);
    exportTextButton_->setEnabled(false);
    runner_.start(config_, currentState_);
}

void HardwarePage::updateCheckRow(const SelfTestCheck& check) {
    QTreeWidgetItem* row = nullptr;
    for (int index = 0; index < resultTree_->topLevelItemCount(); ++index) {
        QTreeWidgetItem* candidate = resultTree_->topLevelItem(index);
        if (candidate->data(0, Qt::UserRole).toString() == check.id) {
            row = candidate;
            break;
        }
    }
    if (row == nullptr) {
        row = new QTreeWidgetItem(resultTree_);
        row->setData(0, Qt::UserRole, check.id);
    }
    const QString status = check.outcome == SelfTestOutcome::Passed
        ? QStringLiteral("✅ 通过")
        : (check.outcome == SelfTestOutcome::Warning ? QStringLiteral("⚠️ 警告") : QStringLiteral("❌ 失败"));
    row->setText(0, status);
    row->setText(1, check.name);
    row->setText(2, check.detail);
    row->setText(3, check.suggestion);
    row->setText(4, QStringLiteral("%1 ms").arg(check.durationMs));
    row->setToolTip(2, check.detail);
    row->setToolTip(3, check.suggestion);
}

void HardwarePage::finishFullCheck(const SelfTestReport& report) {
    latestReport_ = report;
    runButton_->setEnabled(true);
    runButton_->setText(QStringLiteral("重新完整检查"));
    exportJsonButton_->setEnabled(true);
    exportTextButton_->setEnabled(true);
    reportSummary_->setText(
        QStringLiteral("%1 总体结果：%2\n报告已自动保存：%3")
            .arg(report.summary(), selfTestOutcomeText(report.overallOutcome()), report.jsonPath));
}

void HardwarePage::exportReport(bool jsonFormat) {
    if (latestReport_.checks.isEmpty()) {
        return;
    }
    const QString extension = jsonFormat ? QStringLiteral("json") : QStringLiteral("txt");
    const QString suggested = QStringLiteral("The-Eye-of-AI-self-test-%1.%2")
                                  .arg(latestReport_.reportId, extension);
    const QString filePath = QFileDialog::getSaveFileName(
        this,
        jsonFormat ? QStringLiteral("导出 JSON 验收报告") : QStringLiteral("导出文本验收报告"),
        suggested,
        jsonFormat ? QStringLiteral("JSON 文件 (*.json)") : QStringLiteral("文本文件 (*.txt)"));
    if (filePath.isEmpty()) {
        return;
    }
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        reportSummary_->setText(QStringLiteral("导出失败：%1").arg(file.errorString()));
        return;
    }
    file.write(jsonFormat ? latestReport_.toJson() : latestReport_.toText().toUtf8());
    reportSummary_->setText(QStringLiteral("报告已导出：%1").arg(filePath));
}
