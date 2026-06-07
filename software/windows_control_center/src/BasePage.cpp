#include "BasePage.h"

#include <QLabel>
#include <QVBoxLayout>

BasePage::BasePage(const QString& title, const QString& subtitle, QWidget* parent)
    : QWidget(parent) {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(24, 22, 24, 24);
    root->setSpacing(16);

    title_ = new QLabel(title, this);
    title_->setObjectName("pageTitle");
    subtitle_ = new QLabel(subtitle, this);
    subtitle_->setObjectName("pageSubtitle");
    subtitle_->setWordWrap(true);

    body_ = new QVBoxLayout();
    body_->setSpacing(14);

    root->addWidget(title_);
    root->addWidget(subtitle_);
    root->addLayout(body_, 1);
}

QVBoxLayout* BasePage::bodyLayout() const {
    return body_;
}

QLabel* BasePage::titleLabel() const {
    return title_;
}
