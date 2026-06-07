#include "HardwarePage.h"

#include "StatusCard.h"

#include <QGridLayout>

HardwarePage::HardwarePage(QWidget* parent)
    : BasePage("Hardware Status", "Connection, service, camera, microphone, Qwen and network checks.", parent) {
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
              {"Raspberry Pi", "Waiting for SSH handshake", HealthLevel::Checking},
              {"NUCLEO", "Waiting for serial scan", HealthLevel::Unknown},
              {"Camera", "Waiting for /dev/video*", HealthLevel::Unknown},
              {"Microphone", "Waiting for arecord -l", HealthLevel::Unknown}
          }
        : state.hardwareItems;

    for (int i = 0; i < items.size(); ++i) {
        const int row = i / 2;
        const int col = i % 2;
        grid_->addWidget(new StatusCard(items[i].name, items[i].detail, items[i].level, this), row, col);
    }
}
