#pragma once

#include "BasePage.h"
#include "ConnectionState.h"

class QGridLayout;

class HardwarePage : public BasePage {
public:
    explicit HardwarePage(QWidget* parent = nullptr);
    void setState(const ConnectionState& state);

private:
    QGridLayout* grid_ = nullptr;
};
