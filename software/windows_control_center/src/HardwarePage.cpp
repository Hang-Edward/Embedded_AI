#include "HardwarePage.h"

#include "StatusCard.h"

#include <QGridLayout>

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

    const QList<HealthItem> items = state.hardwareItems.isEmpty()
        ? QList<HealthItem> {
              {"树莓派", "等待 SSH 握手", HealthLevel::Checking},
              {"NUCLEO", "等待串口扫描", HealthLevel::Unknown},
              {"摄像头", "等待 /dev/video* 检测", HealthLevel::Unknown},
              {"麦克风", "等待 arecord -l 检测", HealthLevel::Unknown}
          }
        : state.hardwareItems;

    HealthLevel buttonLevel = HealthLevel::Checking;
    if (state.assistantStatus == AssistantStatus::Ready) {
        buttonLevel = HealthLevel::Ok;
    } else if (state.assistantStatus == AssistantStatus::Error || state.assistantStatus == AssistantStatus::Offline) {
        buttonLevel = HealthLevel::Error;
    } else if (state.assistantStatus == AssistantStatus::Warning) {
        buttonLevel = HealthLevel::Warning;
    }

    QList<HealthItem> displayItems;
    displayItems << HealthItem{
        "蓝色按钮状态",
        state.assistantStatusText.isEmpty() ? "等待状态刷新" : state.assistantStatusText,
        buttonLevel
    };
    displayItems.append(items);

    for (int i = 0; i < displayItems.size(); ++i) {
        const int row = i / 2;
        const int col = i % 2;
        grid_->addWidget(new StatusCard(displayItems[i].name, displayItems[i].detail, displayItems[i].level, this), row, col);
    }
}
