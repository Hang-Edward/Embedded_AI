#include "Theme.h"

QString Theme::styleSheet() {
    return R"QSS(
* {
    font-family: "Microsoft YaHei UI", "Segoe UI", sans-serif;
    font-size: 14px;
}
QMainWindow, QWidget#central {
    color: #edf6ff;
    background: #050b19;
    border-image: url(:/assets/liquid_space_wallpaper.png) 0 0 0 0 stretch stretch;
}
QWidget#sidebar {
    background: rgba(9, 18, 38, 178);
    border-right: 1px solid rgba(180, 220, 255, 55);
}
QWidget#glassHeader, QWidget#glassPanel, QWidget#statusCard, QWidget#messageBubble {
    background: rgba(13, 26, 54, 172);
    border: 1px solid rgba(185, 222, 255, 52);
    border-radius: 18px;
}
QPushButton {
    min-height: 34px;
}
QPushButton#navButton {
    background: rgba(255, 255, 255, 18);
    color: #c8daf6;
    border: 1px solid rgba(255, 255, 255, 24);
    border-radius: 13px;
    padding: 11px 14px;
    text-align: left;
}
QPushButton#navButton:hover {
    background: rgba(95, 165, 255, 58);
    color: white;
    border-color: rgba(170, 220, 255, 95);
}
QPushButton#navButton[active="true"] {
    background: rgba(74, 144, 255, 116);
    color: white;
    border-color: rgba(205, 235, 255, 135);
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
    background: rgba(25, 45, 86, 156);
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
    background: rgba(5, 13, 30, 150);
    border: 1px solid rgba(155, 210, 255, 50);
    border-radius: 18px;
    color: #c7d9ef;
}
QTextEdit#logView, QLineEdit, QComboBox {
    background: rgba(8, 18, 38, 172);
    color: #ecf6ff;
    border: 1px solid rgba(170, 220, 255, 58);
    border-radius: 14px;
    padding: 10px;
    selection-background-color: rgba(83, 160, 255, 165);
}
QListWidget#recentList {
    background: rgba(8, 18, 38, 160);
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
