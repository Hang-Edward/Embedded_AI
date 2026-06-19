#include "Theme.h"

QString Theme::styleSheet() {
    return R"QSS(
* {
    font-family: "Microsoft YaHei UI", "Segoe UI", sans-serif;
    font-size: 14px;
}
QMainWindow {
    color: #edf6ff;
    background: #050b19;
}
QWidget#central, QStackedWidget#pageStack, QStackedWidget#pageStack > QWidget {
    background: transparent;
    border: none;
}
QWidget#statusCard {
    background: rgba(19, 39, 76, 108);
    border: 1px solid rgba(215, 239, 255, 62);
    border-radius: 16px;
}
QWidget#messageBubble {
    background: rgba(5, 13, 30, 218);
    border: 1px solid rgba(133, 191, 238, 72);
    border-radius: 16px;
}
QPushButton {
    min-height: 34px;
}
QPushButton#navButton {
    min-height: 52px;
    max-height: 52px;
    background: transparent;
    border: none;
    padding: 0;
}
QPushButton#primaryButton {
    background: rgba(72, 145, 255, 138);
    color: white;
    border: 1px solid rgba(210, 235, 255, 90);
    border-radius: 13px;
    padding: 9px 15px;
    font-weight: 700;
}
QPushButton#primaryButton:hover {
    background: rgba(98, 170, 255, 170);
}
QPushButton#primaryButton:pressed {
    background: rgba(45, 110, 220, 190);
    padding-top: 10px;
    padding-left: 16px;
}
QPushButton#secondaryButton {
    min-height: 38px;
    background: rgba(255, 255, 255, 14);
    color: #dcecff;
    border: 1px solid rgba(195, 229, 255, 48);
    border-radius: 12px;
    padding: 8px 14px;
}
QPushButton#secondaryButton:hover {
    background: rgba(91, 163, 255, 62);
    border-color: rgba(198, 232, 255, 92);
}
QPushButton#secondaryButton:pressed {
    background: rgba(60, 126, 225, 104);
}
QGroupBox#settingsGroup {
    color: #f3f8ff;
    font-weight: 800;
    background: rgba(9, 24, 52, 72);
    border: 1px solid rgba(205, 234, 255, 45);
    border-radius: 16px;
    margin-top: 12px;
    padding: 16px 12px 12px 12px;
}
QGroupBox#settingsGroup::title {
    subcontrol-origin: margin;
    left: 16px;
    padding: 0 7px;
    color: #dcecff;
}
QLabel#appTitle {
    font-size: 20px;
    font-weight: 800;
    color: #ffffff;
}
QLabel#connectionTitle {
    font-size: 18px;
    font-weight: 800;
    color: #ffffff;
}
QWidget#statusColumn {
    background: transparent;
}
QLabel#heroEyebrow {
    color: rgba(213, 235, 255, 0.78);
    font-size: 12px;
    font-weight: 700;
    letter-spacing: 0.5px;
    text-transform: uppercase;
}
QLabel#connectionSubtitle, QLabel#pageSubtitle, QLabel#cardDetail {
    color: #bfd0e8;
}
QLabel#pageTitle {
    font-size: 28px;
    font-weight: 900;
    color: #ffffff;
}
QLabel#statusColumnTitle {
    font-size: 20px;
    font-weight: 850;
    color: #ffffff;
}
QLabel#statusColumnSubtitle, QLabel#statusHint {
    color: #bfd0e8;
    line-height: 145%;
}
QLabel#readyText {
    color: #e5f1ff;
    font-weight: 800;
}
QLabel#readyDot {
    border-radius: 8px;
    background: #64748b;
}
QLabel#readyDot[status="ready"] { background: #34d399; }
QLabel#readyDot[status="busy"], QLabel#readyDot[status="warning"] { background: #fbbf24; }
QLabel#readyDot[status="error"] { background: #fb7185; }
QLabel#actionBanner {
    background: rgba(18, 34, 70, 0.58);
    border: 1px solid rgba(170, 215, 255, 48);
    border-radius: 16px;
    color: #e6f3ff;
    font-weight: 800;
    padding: 12px 14px;
}
QLabel#actionBanner[status="ready"] {
    background: rgba(17, 95, 66, 158);
    border-color: rgba(71, 255, 177, 120);
}
QLabel#actionBanner[status="busy"], QLabel#actionBanner[status="warning"] {
    background: rgba(109, 78, 14, 168);
    border-color: rgba(255, 210, 91, 135);
    color: #fff7d6;
}
QLabel#actionBanner[status="error"] {
    background: rgba(110, 24, 40, 178);
    border-color: rgba(255, 128, 150, 145);
    color: #ffe4e8;
}
QLabel#metricChip {
    background: rgba(8, 21, 45, 0.34);
    border: 1px solid rgba(185, 225, 255, 0.14);
    border-radius: 16px;
    padding: 11px 13px;
    color: #e7f4ff;
}
QLabel#metricChip[tone="ready"] {
    background: rgba(18, 84, 61, 0.42);
    border-color: rgba(81, 252, 181, 0.28);
}
QLabel#metricChip[tone="busy"] {
    background: rgba(104, 74, 16, 0.42);
    border-color: rgba(255, 214, 96, 0.30);
}
QLabel#metricChip[tone="warning"] {
    background: rgba(76, 61, 20, 0.38);
    border-color: rgba(255, 210, 112, 0.24);
}
QLabel#metricChip[tone="error"] {
    background: rgba(104, 24, 42, 0.42);
    border-color: rgba(255, 132, 156, 0.28);
}
QLabel#metricChip[tone="neutral"] {
    background: rgba(24, 47, 92, 0.34);
    border-color: rgba(134, 191, 255, 0.20);
}
QWidget#chatStagePanel {
    background: rgba(10, 24, 52, 0.42);
    border: 1px solid rgba(185, 225, 255, 0.16);
    border-radius: 18px;
}
QLabel#chatStageTitle {
    color: #ffffff;
    font-size: 18px;
    font-weight: 850;
}
QLabel#chatStageStatus {
    color: #dcecff;
    font-size: 14px;
    font-weight: 650;
    line-height: 145%;
}
QLabel#chatStageMeta {
    color: #9fb8d7;
    font-size: 12px;
    line-height: 150%;
}
QLabel#cardTitle, QLabel#messageTitle {
    font-weight: 800;
    color: #ffffff;
}
QLabel#messageBody {
    color: #dcecff;
    line-height: 150%;
}
QLabel#avatar {
    background: rgba(75, 150, 255, 148);
    color: white;
    border: 1px solid rgba(218, 240, 255, 105);
    border-radius: 21px;
    font-weight: 800;
}
QLabel[level="ok"] { background: #34d399; border-radius: 6px; }
QLabel[level="warn"] { background: #fbbf24; border-radius: 6px; }
QLabel[level="error"] { background: #fb7185; border-radius: 6px; }
QLabel[level="checking"], QLabel[level="unknown"] { background: #94a3b8; border-radius: 6px; }
QLabel#cameraPreview, QLabel#messageImage {
    background: rgba(5, 13, 30, 92);
    border: 1px solid rgba(155, 210, 255, 50);
    border-radius: 18px;
    color: #c7d9ef;
}
QTextEdit#logView, QLineEdit, QComboBox {
    background: rgba(8, 18, 38, 118);
    color: #ecf6ff;
    border: 1px solid rgba(170, 220, 255, 58);
    border-radius: 14px;
    padding: 10px;
    selection-background-color: rgba(83, 160, 255, 165);
}
QLineEdit, QComboBox {
    min-height: 38px;
    max-height: 38px;
    padding: 2px 12px;
}
QComboBox::drop-down {
    width: 34px;
    border: none;
}
QListWidget#recentList {
    background: rgba(8, 18, 38, 108);
    color: #e6f3ff;
    border: 1px solid rgba(170, 220, 255, 55);
    border-radius: 16px;
    padding: 8px;
}
QListWidget#recentList::item {
    padding: 10px;
    border-radius: 11px;
}
QListWidget#recentList::item:selected {
    background: rgba(74, 144, 255, 145);
    color: white;
}
QScrollArea#chatScroll {
    border: 0;
    background: transparent;
}
QScrollArea#settingsScroll,
QScrollArea#settingsScroll::viewport,
QWidget#settingsContent {
    background: transparent;
    border: none;
}
QScrollArea#chatScroll QWidget,
QScrollArea#chatScroll QAbstractScrollArea,
QScrollArea#chatScroll QAbstractScrollArea::viewport {
    background: transparent;
}
QScrollBar:vertical {
    background: rgba(4, 12, 29, 105);
    width: 13px;
    margin: 5px 2px 5px 2px;
    border: 1px solid rgba(151, 207, 255, 28);
    border-radius: 6px;
}
QScrollBar::handle:vertical {
    min-height: 44px;
    background: rgba(117, 187, 255, 132);
    border: 1px solid rgba(220, 243, 255, 92);
    border-radius: 5px;
}
QScrollBar::handle:vertical:hover {
    background: rgba(143, 207, 255, 185);
    border-color: rgba(235, 249, 255, 145);
}
QScrollBar::handle:vertical:pressed {
    background: rgba(91, 164, 242, 218);
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0px;
    background: transparent;
    border: none;
}
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
    background: transparent;
}
)QSS";
}
