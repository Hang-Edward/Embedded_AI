#include "HardwarePage.h"

#include "StatusCard.h"

#include <QGridLayout>

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
        return "绿灯：系统就绪，可以按 NUCLEO 蓝色按钮触发语音输入。";
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

HardwarePage::HardwarePage(QWidget* parent)
    : BasePage("连接诊断", "检查树莓派网络、SSH、服务、NUCLEO、摄像头、麦克风、Qwen 配置和 API 网络。", parent) {
    auto* container = new QWidget(this);
    grid_ = new QGridLayout(container);
    grid_->setContentsMargins(0, 0, 0, 0);
    grid_->setSpacing(14);
    bodyLayout()->addWidget(container);
}

void HardwarePage::setState(const ConnectionState& state) {
    while (auto* item = grid_->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    QList<HealthItem> displayItems;
    displayItems << HealthItem{"旋钮 / 按钮触发", assistantDetail(state), assistantLevel(state.assistantStatus)};
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
