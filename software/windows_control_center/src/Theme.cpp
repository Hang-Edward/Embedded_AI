#include "Theme.h"

QString Theme::styleSheet() {
    return R"QSS(
* {
    font-family: "Microsoft YaHei UI", "Segoe UI", sans-serif;
    font-size: 14px;
}
QMainWindow, QWidget#central {
    background: #0b1020;
    color: #e5eefc;
}
QWidget#sidebar {
    background: #111827;
    border-right: 1px solid #263247;
}
QPushButton#navButton {
    background: transparent;
    color: #b7c5dc;
    border: 0;
    border-radius: 8px;
    padding: 12px 14px;
    text-align: left;
}
QPushButton#navButton:hover {
    background: #1d2a3d;
    color: white;
}
QPushButton#navButton[active="true"] {
    background: #2563eb;
    color: white;
}
QLabel#appTitle {
    font-size: 20px;
    font-weight: 700;
}
QLabel#connectionTitle {
    font-size: 17px;
    font-weight: 700;
}
QLabel#connectionSubtitle, QLabel#pageSubtitle, QLabel#cardDetail {
    color: #93a4bd;
}
QLabel#readyText {
    color: #dbeafe;
    font-weight: 700;
}
QLabel#readyDot {
    border-radius: 8px;
    background: #64748b;
}
QLabel#readyDot[status="ready"] {
    background: #22c55e;
}
QLabel#readyDot[status="busy"], QLabel#readyDot[status="warning"] {
    background: #f59e0b;
}
QLabel#readyDot[status="error"] {
    background: #ef4444;
}
QLabel#actionBanner {
    background: #13213a;
    border: 1px solid #2f4f7a;
    border-radius: 10px;
    color: #dbeafe;
    font-weight: 700;
    padding: 10px 14px;
}
QLabel#actionBanner[status="ready"] {
    background: #123322;
    border-color: #22c55e;
}
QLabel#actionBanner[status="busy"], QLabel#actionBanner[status="warning"] {
    background: #352810;
    border-color: #f59e0b;
    color: #fff7cc;
}
QLabel#actionBanner[status="error"] {
    background: #3a1518;
    border-color: #ef4444;
    color: #fee2e2;
}
QLabel#pageTitle {
    font-size: 26px;
    font-weight: 800;
    color: #f8fbff;
}
QWidget#statusCard, QWidget#messageBubble {
    background: #111c2f;
    border: 1px solid #25344d;
    border-radius: 10px;
}
QLabel#cardTitle, QLabel#messageTitle {
    font-weight: 700;
    color: #f8fbff;
}
QLabel#messageBody {
    color: #dce8f8;
    line-height: 150%;
}
QLabel#avatar {
    background: #1e40af;
    color: white;
    border-radius: 21px;
    font-weight: 700;
}
QPushButton#primaryButton {
    background: #2563eb;
    color: white;
    border: 0;
    border-radius: 8px;
    padding: 10px 16px;
    font-weight: 700;
}
QPushButton#primaryButton:hover {
    background: #3b82f6;
}
QLabel[level="ok"] {
    background: #22c55e;
    border-radius: 6px;
}
QLabel[level="warn"] {
    background: #f59e0b;
    border-radius: 6px;
}
QLabel[level="error"] {
    background: #ef4444;
    border-radius: 6px;
}
QLabel[level="checking"], QLabel[level="unknown"] {
    background: #64748b;
    border-radius: 6px;
}
QLabel#cameraPreview {
    background: #08111f;
    border: 1px dashed #335071;
    border-radius: 12px;
    color: #94a3b8;
}
QTextEdit#logView, QLineEdit, QComboBox {
    background: #0f172a;
    color: #e2e8f0;
    border: 1px solid #334155;
    border-radius: 8px;
    padding: 8px;
}
QListWidget#recentList {
    background: #0f172a;
    color: #dbeafe;
    border: 1px solid #334155;
    border-radius: 10px;
    padding: 8px;
}
QListWidget#recentList::item {
    padding: 10px;
    border-radius: 8px;
}
QListWidget#recentList::item:selected {
    background: #2563eb;
    color: white;
}
QScrollArea#chatScroll {
    border: 0;
    background: transparent;
}
)QSS";
}
