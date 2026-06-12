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
QWidget#statusCard, QWidget#messageBubble {
    background: rgba(19, 39, 76, 108);
    border: 1px solid rgba(215, 239, 255, 62);
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
QLabel#connectionSubtitle, QLabel#pageSubtitle, QLabel#cardDetail {
    color: #bfd0e8;
}
QLabel#pageTitle {
    font-size: 28px;
    font-weight: 900;
    color: #ffffff;
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
    background: rgba(25, 45, 86, 116);
    border: 1px solid rgba(170, 215, 255, 64);
    border-radius: 16px;
    color: #e6f3ff;
    font-weight: 800;
    padding: 11px 15px;
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
    background: transparent;
    width: 10px;
}
QScrollBar::handle:vertical {
    background: rgba(175, 220, 255, 75);
    border-radius: 5px;
}
)QSS";
}
