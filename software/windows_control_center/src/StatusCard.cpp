#include "StatusCard.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QVBoxLayout>

StatusCard::StatusCard(QWidget* parent)
    : StatusCard("Unknown", "Waiting for data", HealthLevel::Unknown, parent) {
}

StatusCard::StatusCard(const QString& title, const QString& detail, HealthLevel level, QWidget* parent)
    : QWidget(parent) {
    setObjectName("statusCard");
    auto* root = new QHBoxLayout(this);
    root->setContentsMargins(16, 14, 16, 14);
    root->setSpacing(12);

    indicator_ = new QLabel(this);
    indicator_->setFixedSize(12, 12);

    auto* textBox = new QVBoxLayout();
    textBox->setSpacing(4);
    title_ = new QLabel(this);
    title_->setObjectName("cardTitle");
    detail_ = new QLabel(this);
    detail_->setObjectName("cardDetail");
    detail_->setWordWrap(true);
    textBox->addWidget(title_);
    textBox->addWidget(detail_);

    root->addWidget(indicator_, 0, Qt::AlignTop);
    root->addLayout(textBox, 1);
    setStatus(title, detail, level);
}

void StatusCard::setStatus(const QString& title, const QString& detail, HealthLevel level) {
    title_->setText(title);
    detail_->setText(detail);
    indicator_->setProperty("level", healthLevelClass(level));
    indicator_->style()->unpolish(indicator_);
    indicator_->style()->polish(indicator_);
}
